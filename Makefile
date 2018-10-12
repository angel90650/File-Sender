#Makefile

all: tcp udp

tcp: tcp_server tcp_client

udp: udp_server udp_client

tcp_server: 
	gcc -g -o tcpserver server_c_tcp.c

tcp_client: 
	gcc -g -o tcpclient client_c_tcp.c

udp_client: 
	gcc -g -o udpclient client_c_udp.c

udp_server:
	gcc -g -o udpserver server_c_udp.c

clean:
	rm -f tcpserver udpclient udpserver tcpclient
