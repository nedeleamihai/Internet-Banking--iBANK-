build: server client

server:	server.cpp
	g++ -o server -lnsl server.cpp

client:	client.cpp
	g++ -o client -lnsl client.cpp

clean:
	rm -f *.o *~
	rm -f server client
	rm *.log


