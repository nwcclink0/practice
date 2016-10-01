CC=clang

all: epoll-client epoll-server epoll-client-multi-server

epoll-client.o: epoll-client.c
	$(CC) -c -o epoll-client.o epoll-client.c

epoll-client: epoll-client.o
	$(CC) -o epoll-client epoll-client.o

epoll-client-multi-server.o: epoll-client-multi-server.c
	$(CC) -c -o epoll-client-multi-server.o epoll-client-multi-server.c

epoll-client-multi-server: epoll-client-multi-server.o
	$(CC) -o epoll-client-multi-server epoll-client-multi-server.o

epoll-server.o: epoll-server.c
	$(CC) -c -o epoll-server.o epoll-server.c

epoll-server: epoll-server.o
	$(CC) -o epoll-server epoll-server.o

test_client: epoll-client
	./epoll-client 127.0.0.1 3000

test_server: epoll-server
	./epoll-server 3000

astyle:
	astyle  --style=kr --recursive "*.c" 

clean:
	rm -rf *.o epoll-client epoll-server epoll-client-multi-server

