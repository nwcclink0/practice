CC=clang

all: epoll-client

epoll-client.o: epoll-client.c
	$(CC) -c -o epoll-client.o epoll-client.c

epoll-client: epoll-client.o
	$(CC) -o epoll-client epoll-client.o

test_localhost: example-client
	./example-client 127.0.0.1 3000

clean:
	rm -rf *.o epoll-client
