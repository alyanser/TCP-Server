# TCP echo server
Asynchronous TCP echo server implementation using Asio library and SSL/TLS.

<b>Testing</b>:<br>
<pre>
git clone https://github.com/alyanser/TCP-Server
cd TCP-Server/scripts
bash generate_certs.sh && bash build.sh && bash run.sh
// different terminal
openssl s_client -connect localhost:1234
</pre>
