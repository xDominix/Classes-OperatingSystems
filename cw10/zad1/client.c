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

void error(char* name ){ printf("%s\n",name);exit(1);}
struct sockaddr_un localSocket;
struct sockaddr_in webSocket;
int conFd;
char recvbuf[BUF_SIZE];
int playing = 0;

typedef enum{ CIRCLE_WINS, CROSS_WINS, DRAW, KEEP_PLAYING } gameStatus;
typedef enum{  WEB, LOCAL }connectionType;

void sigintHandler(int signo){ puts("\nExiting..."); exit(0); }
void sendGameOver(void){ if(playing) write(conFd, "DC", strlen("DC")+1);}	

void drawTable(const char * table)
{
    char tableContent[500];
    sprintf(tableContent, "%c|%c|%c\n%c|%c|%c\n%c|%c|%c\n", table[0], table[1], table[2], table[3], table[4], table[5], table[6], table[7], table[8]);
    printf("%s", tableContent);
}

gameStatus checkVictory(const char * table)
{
    gameStatus  result = KEEP_PLAYING;
    for(int i = 0; i < 3; i++)
    {
        if (table[i*3] == table[i*3+1] && table[i*3+1] == table[i*3+2])
        {
            if(table[i*3] == 'O') result = CIRCLE_WINS;
            else result = CROSS_WINS;
            break;
        }
        if(table[i] == table[i+3] && table[i+3] == table[i+6])
        {
            if(table[i] == 'O')  result = CIRCLE_WINS;
            else  result = CROSS_WINS;
            break;
        }
    }

    if(table[0] == table[4] && table[4] == table[8])
    {
        if(table[0] == 'O') result = CIRCLE_WINS;
        else if (table[0] == 'X') result = CROSS_WINS;
    }
    if(table[2] == table[4] && table[4] == table[6])
    {
        if(table[4] == 'O') result = CIRCLE_WINS;
        else if (table[0] == 'X') result = CROSS_WINS;
    }

    if(result == KEEP_PLAYING){
        result = DRAW;
        for(int i = 0; i < 9; i++)
        {
            if(table[i] != 'O' || table[i]!='X')
            {
                result = KEEP_PLAYING;
                break;
            }
        }
    }

    return result;
}

void pickPlace(char * table, char myChar){
    while(1) 
    {
        int position;
        puts("Pick position (1-9)");
        scanf("%d", &position);
        if (position < 1 || position > 9) 
        {
            puts("Invalid position selected");
            continue;
        }
        if (table[position - 1] == 'O' || table[position-1]=='X') 
        {
            puts("Position already taken");
            continue;
        }
        table[position - 1] = myChar;
        break;
    }
}

int parseVictory(char * table)
{
    gameStatus vt = checkVictory(table);
    if(vt == KEEP_PLAYING) return 0;
    else if(vt == CIRCLE_WINS) puts("Circle wins!");
    else if(vt == CROSS_WINS) puts("Circle wins!");
    else if(vt == DRAW) puts("Draw!");
    return 1;
}

void runClient(char * myName)
{
    write(conFd, myName, strlen(myName)+1);
    int nread = read(conFd, recvbuf, BUF_SIZE);
    recvbuf[nread] = 0;

    if(strcmp(recvbuf, "TAKEN") == 0) error("Name already taken");
    puts("Connected!");

    while(1)
    {
        nread = read(conFd, recvbuf, BUF_SIZE);
        if(nread < 1) error("read");
        
        recvbuf[nread] = 0;
        if(strcmp(recvbuf, "PING") == 0) write(conFd, "PONG", strlen("PONG")+1);
        else
        {
            playing = 1;
            char table[9] = "123456789";
            char myChar = recvbuf[0];
            int skipFirst = myChar == 'O' ? 0 : 1;
            puts(recvbuf);
            while(1)
            {
                //get move from this player
                if(!skipFirst){
                    puts("YOUR TURN");
                    drawTable(table);
                    int victorious = parseVictory(table);
                    if(victorious){
                        write(conFd, "GAMEOVER", strlen("GAMEOVER")+1);
                        playing = 0;
                        break;
                    }
                    pickPlace(table, myChar);
                    write(conFd, table, 9);
                    drawTable(table);
                    victorious = parseVictory(table);
                    if(victorious){
                        write(conFd, "GAMEOVER", strlen("GAMEOVER")+1);
                        playing = 0;
                        break;
                    }
                }
                else skipFirst = 0;
                
                puts("RIVAL TURN");
                nread = read(conFd, recvbuf, BUF_SIZE);
                if(nread < 1) error("read2");
                if(strcmp(recvbuf, "DC") == 0)
                {	
                    puts("The other player quit the game...");
                    playing = 0;
                    break;
                }
                strncpy(table, recvbuf, 9);
            }
        }
    }
}

int main(int argc, char ** argv)
{
    signal(SIGPIPE, SIG_IGN);
    if(argc < 4) perror("Wrong no arguments");
    
    char * myName = argv[1];
    connectionType ctype;
    if(strcmp(argv[2], "WEB") == 0) ctype = WEB;
    else if(strcmp(argv[2], "LOCAL") == 0) ctype = LOCAL;
    else error("Wrong type (WEB/LOCAL)");
    char * addr = argv[3];
    char * port = NULL;

    if(ctype == WEB)
    {
    	for(int i = 0; i < strlen(argv[3]); i++)
        {
            if(addr[i] == ':')
            {
                addr[i] = 0x0;
                port = addr+i+1;
            }
        }
        if(port == NULL) error("Port number not specified");
        conFd = socket(AF_INET, SOCK_STREAM, 0);
        if(conFd == -1)  error("conFd");
        struct in_addr inAddr;
        if(inet_pton(AF_INET, addr, &inAddr) == 0) error("inet_pton");
        short portnum;
        if((portnum = atoi(port)) == 0) error("portnum");

        webSocket.sin_port = htons(portnum);
        webSocket.sin_addr = inAddr;
        webSocket.sin_family = AF_INET;

        if(connect(conFd, (struct sockaddr*)&webSocket, sizeof(webSocket)) == -1) error("connect");
    }
    else
    {
        conFd = socket(AF_LOCAL, SOCK_STREAM, 0);
        if(conFd == -1) error("conFd2");

        localSocket.sun_family = AF_LOCAL;
        strcpy(localSocket.sun_path, addr);
        if(connect(conFd, (struct sockaddr*)&localSocket, sizeof(localSocket)) == -1) error("connect2");
    }
    atexit(sendGameOver);
    signal(SIGINT, sigintHandler);
    
    runClient(myName);
    return 0;
}
