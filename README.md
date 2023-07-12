Chat entre Cliente e Servidor usando TCP/IP.
O chat é assíncrono, isto é, cada cliente pode enviar quantas mensagens quiser em sequência, além de poderem se conectar vários clientes.
O funcionamento é simples. Existe um servidor e vários clientes. As mensagens dos clientes são enviadas para o servidor que, por sua vez, envia em broadcast a mensagem para todos os outros clientes conectados.

Observação: o código foi feito no Sistema Operacional Windows. Para Linux, o código deverá passar por breves alterações para funcionar corretamente.
