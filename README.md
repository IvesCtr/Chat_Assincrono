# Chat Assíncrono
Chat entre Cliente e Servidor usando <strong>TCP/IP</strong>.
<br><br>
O chat é assíncrono, isto é, cada cliente pode enviar quantas mensagens quiser em sequência, além de ser permitido que vários clientes se conectem.
<br>
O funcionamento é simples. Existe um servidor e vários clientes. As mensagens dos clientes são enviadas para o servidor que, por sua vez, envia em broadcast a mensagem para todos os outros clientes conectados.
<br><br>
<strong>Observação:</strong> o código foi feito no <strong>Sistema Operacional Windows</strong>. Para Linux, o código deverá passar por breves alterações para funcionar corretamente, pois as bibliotecas são levemente diferentes. 
