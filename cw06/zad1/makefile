compile:
	make clean
	gcc -Wall -o server server.c
	gcc -Wall -o client client.c

clean:
	rm -f *.o
	rm -f *.a
	rm -f server
	rm -f client

all:
	make compile
	./server
	
