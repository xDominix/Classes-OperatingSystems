#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include "mylib.h"

int qid=-1; //queue id
int cids[10] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};//client ids
int counter = 0;

void handler(int signum) { printf("\nOdebrano SIGINT\n"); exit(0); }

void login(my_msg* msg){
    cids[counter] = msg->client_id;
    if (counter >= MAX_CLIENTS)
    {
        printf("Max clients reached.\n");
        sprintf(msg->mtext, "%d", -1);
    }
    else
    {
        sprintf(msg->mtext, "%d", counter);
        while (cids[counter] > 0 && counter < MAX_CLIENTS) counter++;
    }
    if (msgsnd(cids[counter-1], msg, MSG_SIZE, 0) == -1)   error("login");
}

void stop(my_msg * msg)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (cids[i] == msg->client_id) { cids[i] = -1; counter = i;  }
    }
}

void list()
{
    printf("Clients list:\n");
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (cids[i] > 0) printf(" - %d\n", cids[i]);
    }
}

void to_all(my_msg * msg)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (cids[i] > 0){ if (msgsnd(cids[i], msg, MSG_SIZE, 0) == -1) error("to_all");}
    }
}

void to_one(my_msg * msg)
{
    if (msgsnd(msg->to_id, msg, MSG_SIZE, 0) == -1)     error("to_one");
}

void proceed(my_msg * msg)
{
    if (!msg) return;
    switch(msg->mtype){
        case LOGIN:
            login(msg);
            printf("Client %d joined.\n", msg->client_id);
        break;
        case STOP:
            stop(msg);
            printf("Client %d left.\n", msg->client_id);
        break;
        case TOALL:
            to_all(msg);
            printf("Message '%s' to all.\n", msg->mtext);
        break;
        case TOONE:
            to_one(msg);
            printf("Message '%s' to %d.\n", msg->mtext, msg->to_id);
        break;
        case LIST:
            list();
        break;
    }
}

void delete_queue()
{
    if (qid > -1)
    {
        int tmp = msgctl(qid, IPC_RMID, NULL);
        if (tmp == -1) printf("delete_queue");
        printf("Queue deleted successfully.\n");
    }
}
int main(int argc, char ** argv)
{
	atexit(delete_queue);
 	signal(SIGINT, handler);
 	
 	char *path = getenv("HOME"); if (!path) error("getenv");
 	
    key_t key = ftok(path, PROJECT_ID); if (key == -1) error("ftok");
    qid = msgget(key, IPC_CREAT| IPC_EXCL | 0666); if (qid == -1) error("msgget");
    
    my_msg msg;
    while(1)
    {
        if (msgrcv(qid, &msg, MSG_SIZE, 0, 0) < 0) error("msgrcv");
        proceed(&msg);
    }

    return 0;
}
