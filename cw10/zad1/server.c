#include "serverClients.h"
#include <pthread.h>
#include <stdio.h>
#include <sys/un.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>

#define BUF_SIZE 120
#define NNOTRESPONDING 40
#define NNOTRESPONDING_CHECK_DELAY 80

serverClientsList * clientsList;
pthread_mutex_t clientsListMutex = PTHREAD_MUTEX_INITIALIZER;
struct sockaddr_in webSockStruct;
struct sockaddr_un localSockStruct;
int unixSocket;
int webSocket;

void error(char* name ){ printf("%s\n",name);exit(1);}
void * handleIncomingConnections(void * dummy)
{
    puts("Starting to listen");
    listen(unixSocket, 12);
    listen(webSocket, 12);

    while(1)
    {
        int cliDesc = -1;
        ClientConnectionType contype = NONE;
        if((cliDesc = accept(webSocket, NULL, NULL)) != -1)
        {
        	puts("Handling web connection.");
            contype = WEB;
        }
        else if((cliDesc = accept(unixSocket, NULL, NULL)) != -1)
        {
            puts("Handling local connection.");
            contype = LOCAL;
        }
        
        char userName[BUF_SIZE];
        if(contype != NONE)
        {
            printf("New client with fd %d\n", cliDesc);
            int nread = read(cliDesc, userName, BUF_SIZE);
            userName[nread] = 0;
            printf("Client requested name %s\n", userName);
            pthread_mutex_lock(&clientsListMutex);
            if(cliList_find(clientsList, userName) != NULL)
            {
                send(cliDesc, "TAKEN", strlen("TAKEN")+1, MSG_DONTWAIT);
                shutdown(cliDesc, SHUT_RDWR);
                close(cliDesc);
            }
            else
            {
                struct sockaddr empt;
                cliList_insert(&clientsList, userName, contype, empt, cliDesc);
                send(cliDesc, "OK", strlen("OK")+1, MSG_DONTWAIT);
            }
            pthread_mutex_unlock(&clientsListMutex);
        }
    }
    return NULL;
}

int getTimeMs() { struct timespec spec; clock_gettime(1, &spec); return spec.tv_sec * 1000 + spec.tv_nsec / 1e6; }
void * handleGames(void * dummy)
{
    char recvbuf[BUF_SIZE];
    int lastTime = getTimeMs();
    while(1) 
    {
        pthread_mutex_lock(&clientsListMutex);
        serverClientsList * client = clientsList;
        while (client != NULL) 
        {
            if (client->connectedToName != NULL) 
            {
                int msgsize = recv(client->confd, recvbuf, BUF_SIZE, MSG_DONTWAIT);
                if(msgsize > 0)
                {
                    recvbuf[msgsize] = 0;
                    serverClientsList * connectedTo = cliList_find(clientsList, client->connectedToName);
                    if (strcmp(recvbuf, "GAMEOVER") == 0) client->connectedToName = NULL;
                    else if(strcmp(recvbuf, "DC") == 0)
                    {
                        send(connectedTo->confd, recvbuf, msgsize, MSG_DONTWAIT);
                        client->connectedToName = NULL;
                        connectedTo->connectedToName = NULL;
                        shutdown(client->confd, SHUT_RDWR);
                        close(client->confd);
                        cliList_delete(&clientsList, client->clientName);
                        break;
                    }
                    else if(strcmp(recvbuf, "PONG") != 0) 
                    {
                        printf("Received board state from client %s, passing to %s\n", client->clientName, client->connectedToName);
                        send(connectedTo->confd, recvbuf, msgsize, MSG_DONTWAIT);
                    }
                    else client->nNotResponding = 0;
                }
            }
            client = client->next;
        }
        pthread_mutex_unlock(&clientsListMutex);

        serverClientsList * fcl = NULL;
        serverClientsList * scl = NULL;
        pthread_mutex_lock(&clientsListMutex);
        client = clientsList;
        while(client != NULL)
        {
            if(client->connectedToName == NULL)
            {
                if(fcl == NULL) fcl = client;
                else
                {
                	scl = client;
                    break;
                }
            }
            client = client->next;
        }
        if(fcl != NULL && scl != NULL)
        {
            fcl->connectedToName = scl->clientName;
            scl->connectedToName = fcl->clientName;
            sprintf(recvbuf, "X, YOU ARE X.\n");
            send(scl->confd, recvbuf, strlen(recvbuf)+1, MSG_DONTWAIT);
           	sprintf(recvbuf, "O, YOU ARE O.\n");
            send(fcl->confd, recvbuf, strlen(recvbuf)+1, MSG_DONTWAIT);
            printf("Paired %s with %s\n", fcl->clientName, scl->clientName);
        }
        pthread_mutex_unlock(&clientsListMutex);

        if(getTimeMs() -lastTime > NNOTRESPONDING_CHECK_DELAY)
        {
            lastTime = getTimeMs();
            pthread_mutex_lock(&clientsListMutex);
            client = clientsList;
            while(client != NULL)
            {
                if(client->connectedToName == NULL) 
                {
                    int nsent = send(client->confd, "PING", strlen("PING") + 1, MSG_DONTWAIT);
                    if(nsent  == -1) client->nNotResponding = NNOTRESPONDING+1;
                    client->nNotResponding++;
                    if (recv(client->confd, recvbuf, BUF_SIZE, MSG_DONTWAIT) > 0) 
                    {
                        recvbuf[strlen("PONG")] = 0;
                        if (strcmp(recvbuf, "PONG") == 0) client->nNotResponding = 0;
                    }
                    else if (client->nNotResponding > NNOTRESPONDING) 
                    {
                        printf("Client %s is not responding. Closing connection.\n", client->clientName);
                        shutdown(client->confd, SHUT_RDWR);
                        close(client->confd);
                        cliList_delete(&clientsList, client->clientName);
                        break;
                    }
                }
                client = client->next;
            }

            pthread_mutex_unlock(&clientsListMutex);
        }
    }
    return NULL;
}

int main(int argc, char ** argv)
{
    signal(SIGPIPE, SIG_IGN);
    clientsList = cliList_create();
    if(argc < 3) error("Wrong no arguments");

    short portNum;
    char * unixPath = argv[2];
    if((portNum = (short)atoi(argv[1])) == 0) error("portnum");

    if((webSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1) error("websocket");
    if((unixSocket = socket(AF_LOCAL, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1) error("unixsocket");
    unlink(unixPath);
    strcpy(localSockStruct.sun_path, unixPath);
    localSockStruct.sun_family = AF_LOCAL;
    if(bind(unixSocket, (struct sockaddr * )&localSockStruct, sizeof(localSockStruct)) == -1)error("bind");

    webSockStruct.sin_family = AF_INET;
    webSockStruct.sin_port = htons(portNum);
    webSockStruct.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(webSocket, (struct sockaddr * )&webSockStruct, sizeof(webSockStruct) )== -1) error("bind2");

    pthread_t listeningThread;
    pthread_t handlingThread;
    pthread_create(&listeningThread, NULL, handleIncomingConnections, NULL);
    pthread_create(&handlingThread, NULL, handleGames, NULL);
    pthread_join(listeningThread, NULL);
    pthread_join(handlingThread, NULL);

	close(webSocket);
    close(unixSocket);
    return 0;
}
