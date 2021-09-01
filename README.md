# TCP echo server
Asynchronous TCP echo server implementation using Asio library and SSL/TLS.

<b>Building:</b>
<pre>
git clone https://github.com/alyanser/TCP-echo-server
cd TCP-echo-server/scripts
bash generate_certs.sh && bash build.sh && bash run.sh
</pre>
<b>Testing:</b>
<pre>
openssl s_client -connect localhost:1234
</pre>
