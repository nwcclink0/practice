CC=clang

all: epoll_threadpool

epoll_threadpool: epoll_threadpool.o threadpool.o
	$(CC) -o epoll_threadpool -pthread threadpool.o epoll_threadpool.o

epoll_threadpool.o: epoll_threadpool.c
	$(CC) -c -I../threadpool -o epoll_threadpool.o epoll_threadpool.c

threadpool.o: ../threadpool/threadpool.c
	$(CC) -c -o threadpool.o ../threadpool/threadpool.c

clean:
	rm -rf *.o epoll_threadpool





