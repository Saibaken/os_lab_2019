all: tcpclient tcpserver udpclient udpserver

tcpclient: 
	gcc tcpclient.c -o tcpclient 

tcpserver:
	gcc tcpserver.c -o tcpserver 

udpclient:
	gcc udpclient.c -o udpclient 

udpserver:
	gcc udpserver.c -o udpserver 

udp: udpserver udpclient

tcp: tcpclient tcpserver

clean:
	rm tcpclient tcpserver udpclient udpserver