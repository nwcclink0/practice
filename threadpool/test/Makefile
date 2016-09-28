CC=clang

all: test1

test1: test1.o threadpool.o
	$(CC) -o test1 -pthread test1.o threadpool.o

test1.o: test1.c
	$(CC) -c -g -I../ -o test1.o test1.c

threadpool.o: ../threadpool.c
	$(CC) -c -g -o threadpool.o ../threadpool.c

clean:
	rm -rf *.o test1
