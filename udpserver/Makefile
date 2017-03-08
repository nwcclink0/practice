CC=clang

all: udpserver

udpserver.o: udpserver.c
	$(CC) -c -o udpserver.o udpserver.c

udpserver: udpserver.o
	$(CC) -o udpserver udpserver.o

astyle:
	astyle  --style=kr --recursive "*.c" 

clean:
	rm -rf *.o udpserver

