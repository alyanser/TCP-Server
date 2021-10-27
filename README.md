# TCP echo server
Asynchronous TCP echo server implementation using asio library.

<b>usage</b>
<pre>
build
run "generate_certs.sh"
launch server
</pre>
<b>testing:</b>
<pre>
openssl s_client -connect localhost:1234
</pre>
<b>logs snippet:</b>
<pre>
[server] : started with 3 threads 
[server] : acceptor bound to port number 1234 
[server] : listening state 
[server] : new client [ 15670493434874735584 ] attempting to connect. handshake pending 
[server] : handshake attempt with client [ 15670493434874735584 ] 
[server] : listening state 
[server] : handshake successful with client [ 15670493434874735584 ] 
[server] : new client [ 2834833214195675554 ] attempting to connect. handshake pending 
[server] : handshake attempt with client [ 2834833214195675554 ] 
[server] : listening state 
[server] : handshake successful with client [ 2834833214195675554 ] 
[server] : message received from client [ 15670493434874735584 ] 
[server] : message received from client [ 2834833214195675554 ] 
[server] : processing message from client [ 15670493434874735584 ] 

*** message from client [15670493434874735584]

	--- START MESSAGE ---
hello there from first client
first client ending my message
	--- END MESSAGE---

[server] : message received from client [ 15670493434874735584 ] 
[server] : processing message from client [ 15670493434874735584 ] 

[server] : 90 bytes sent to client [ 15670493434874735584 ] 

*** response sent to client [15670493434874735584]

	--- START RESPONSE ---
hello there from first client
first client ending my message

	--- ENDRESPONSE---

[server] : connection closed with client [ 15670493434874735584 ] 
[server] : processing message from client [ 2834833214195675554 ] 
...
</pre>