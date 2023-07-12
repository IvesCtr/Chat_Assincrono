#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#define PORT 5000
#define MAX_CLIENTS 10

typedef struct {
    SOCKET socket;
    HANDLE thread;
    char name[100];
} ClientData;

ClientData clients[MAX_CLIENTS];
int num_clients = 0;
HANDLE send_thread;

void broadcast_message(const char* message, const char* sender_name) {
    char formatted_message[1124];

    if (sender_name == "sistema"){
        sprintf(formatted_message, "%s", message);
    } else {
        sprintf(formatted_message, "%s: %s", sender_name, message);
    }

    for (int i = 0; i < num_clients; i++) {
        if (strcmp(clients[i].name, sender_name) != 0) {
            send(clients[i].socket, formatted_message, strlen(formatted_message), 0);
        }
    }
}

void remove_client(int index) {
    printf("-- %s saiu --\n", clients[index].name);
    char saiu [20];
    sprintf(saiu, "-- %s saiu --\n", clients[index].name);
    broadcast_message(saiu, "sistema");

    // Fechar o socket e a thread do cliente
    closesocket(clients[index].socket);
    CloseHandle(clients[index].thread);

    // Remover o cliente da lista
    for (int i = index; i < num_clients - 1; i++) {
        clients[i] = clients[i + 1];
    }

    num_clients--;
}

DWORD WINAPI receive_messages(LPVOID arg) {
    ClientData* client_data = (ClientData*)arg;
    SOCKET client_socket = client_data->socket;
    char buffer[1024];
    int recv_size;

    while ((recv_size = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[recv_size] = '\0';

        // Verificar se é o nome do cliente
        if (strlen(client_data->name) == 0) {
            strcpy(client_data->name, buffer);
            printf("-- %s entrou --\n", client_data->name);
            char entrou [20];
            sprintf(entrou, "-- %s entrou --\n", client_data->name);
            broadcast_message(entrou, "sistema");
        } else {
            printf("%s: %s\n", client_data->name, buffer);
            broadcast_message(buffer, client_data->name);
        }

        memset(buffer, 0, sizeof(buffer));
    }

    // Cliente desconectado
    int client_index = client_data - clients;
    remove_client(client_index);

    return 0;
}

DWORD WINAPI send_messages(LPVOID arg) {
    while (1) {
        char message[1024];
        fgets(message, sizeof(message), stdin);
        broadcast_message(message, "Servidor");
    }

    return 0;
}

int main() {
    WSADATA wsaData;
    SOCKET sockfd;
    struct sockaddr_in serverAddr;
    int addr_size;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Erro ao inicializar o Winsock.\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        printf("Erro ao abrir o socket.\n");
        exit(1);
    }
    printf("Socket criado com sucesso.\n");

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Erro ao fazer o bind.\n");
        exit(1);
    }
    printf("Bind feito com sucesso.\n");

    if (listen(sockfd, 10) == SOCKET_ERROR) {
        printf("Erro ao escutar.\n");
        exit(1);
    }

    printf("Aguardando conexoes...\n");

    // Cria uma thread para enviar mensagens assíncronas do servidor
    DWORD send_thread_id;
    send_thread = CreateThread(NULL, 0, send_messages, NULL, 0, &send_thread_id);
    if (send_thread == NULL) {
        printf("Erro ao criar a thread de envio de mensagens.\n");
        exit(1);
    }

    while (1) {
        addr_size = sizeof(serverAddr);
        SOCKET client_socket = accept(sockfd, (struct sockaddr*)&serverAddr, &addr_size);
        if (client_socket == INVALID_SOCKET) {
            printf("Erro ao aceitar a conexao.\n");
            continue;
        }

        // Verificar se o limite de clientes foi atingido
        if (num_clients >= MAX_CLIENTS) {
            printf("Limite de clientes atingido. Rejeitando nova conexao.\n");
            closesocket(client_socket);
            continue;
        }

        // Adicionar o cliente à lista
        clients[num_clients].socket = client_socket;
        clients[num_clients].thread = NULL;
        memset(clients[num_clients].name, 0, sizeof(clients[num_clients].name));

        // Criar uma nova thread para lidar com o cliente
        clients[num_clients].thread = CreateThread(NULL, 0, receive_messages, &clients[num_clients], 0, NULL);
        if (clients[num_clients].thread == NULL) {
            printf("Erro ao criar a thread.\n");
            closesocket(client_socket);
            continue;
        }

        num_clients++;
    }

    // Aguardar a conclusão da thread de envio de mensagens
    WaitForSingleObject(send_thread, INFINITE);
    CloseHandle(send_thread);

    // Aguardar a conclusão das threads de clientes
    for (int i = 0; i < num_clients; i++) {
        WaitForSingleObject(clients[i].thread, INFINITE);
        CloseHandle(clients[i].thread);
        closesocket(clients[i].socket);
    }

    closesocket(sockfd);
    WSACleanup();

    return 0;
}
