compile: 
	gcc -Wall -o server server.c -lpthread
	gcc -Wall -o client client.c -lpthread
	
serv:
	./server 33333 serverSocket
	
cli1:
	./client player1 WEB 127.0.0.1:33333

cli2:
	./client player2 WEB 127.0.0.1:33333

loccli1:
	./client player1 LOCAL serverSocket

loccli2:
	./client player2 LOCAL serverSocket

clean:
	rm -f server
	rm -f client
	rm -f serverSocket
