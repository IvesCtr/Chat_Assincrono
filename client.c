#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <winsock2.h>
#include <windows.h>

#define PORT 5000

SOCKET sockfd;
char client_name[100];

DWORD WINAPI receive_messages(LPVOID arg) {
    SOCKET* client_socket = (SOCKET*)arg;
    char buffer[1024];
    int recv_size;

    while ((recv_size = recv(*client_socket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[recv_size] = '\0';
        printf("%s\n", buffer);
        memset(buffer, 0, sizeof(buffer));
    }

    // Servidor desconectado
    closesocket(*client_socket);
    return 0;
}

void disconnect() {
    printf("Desconectando...\n");
    closesocket(sockfd);
    WSACleanup();
    exit(0);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Uso: cliente <nome>\n");
        return 1;
    }

    strcpy(client_name, argv[1]);

    WSADATA wsaData;
    struct sockaddr_in serverAddr;
    SOCKET client_socket;
    HANDLE thread;
    char message[1024];

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
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Erro de conexao.\n");
        exit(1);
    }
    printf("Conectado ao servidor.\n");

    // Enviar o nome do cliente ao servidor
    send(sockfd, client_name, strlen(client_name), 0);

    // Criar uma thread para receber mensagens do servidor
    thread = CreateThread(NULL, 0, receive_messages, &sockfd, 0, NULL);
    if (thread == NULL) {
        printf("Erro ao criar a thread.\n");
        exit(1);
    }

    // Configurar o manipulador de sinal para capturar o sinal SIGINT (Ctrl + C)
    if (signal(SIGINT, disconnect) == SIG_ERR) {
        printf("Erro ao definir o manipulador de sinal.\n");
        exit(1);
    }

    while (1) {
        fgets(message, sizeof(message), stdin);

        // Enviar mensagem ao servidor
        if (send(sockfd, message, strlen(message), 0) == SOCKET_ERROR) {
            printf("Erro ao enviar a mensagem.\n");
            break;
        }

        memset(message, 0, sizeof(message));
    }

    // Aguardar a conclusão da thread de recebimento de mensagens
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);

    closesocket(sockfd);
    WSACleanup();

    return 0;
}
