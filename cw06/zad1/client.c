#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>

#include "mylib.h"

int cid = -1;//client id
int qid = -1;//queue id

void stop()
{
    my_msg msg;
    msg.client_id= cid;
    msg.mtype = STOP;
    sprintf(msg.mtext, "%s", "Stop");
    if (msgsnd(qid, &msg, MSG_SIZE, 0) < 0) error("stop");
    exit(0);
}

void handler(int signum) { printf("Odebrano sygnal SIGINT\n"); stop(); }

void to_all(char * mess)
{
    my_msg msg;
    msg.client_id = cid;
    msg.mtype = TOALL;
    sprintf(msg.mtext, "%s", mess);
    if (msgsnd(qid, &msg, MSG_SIZE, 0) < 0) error("to_all");
}

void to_one(int to_id, char * mess)
{
    my_msg msg;
    msg.client_id = cid;
    msg.to_id=to_id;
    msg.mtype = TOONE;
    sprintf(msg.mtext, "%s", mess);
    if (msgsnd(qid, &msg, MSG_SIZE, 0) < 0) error("to_one");
}

void list()
{
    my_msg msg;
    msg.client_id = cid;
    msg.mtype = LIST;
    sprintf(msg.mtext, "%s", "List");
    if (msgsnd(qid, &msg, MSG_SIZE, 0) < 0) error("list");
}

void proceed(char * cmd, char * rest)
{
    if (strcmp(cmd, "LIST") == 0) list();
    else if (strcmp(cmd, "2ALL") == 0) to_all(rest);
    else if (strcmp(cmd, "STOP") == 0) stop();
    else if (strcmp(cmd, "2ONE") == 0){
    	char delims[3]= {' ', '\n', '\t'};
        char * num = strtok_r(rest, delims, &rest);
        to_one(atoi(num), rest);
    }
	else  printf("Wrong command\n");
}

void delete_queue()
{
    if ( msgctl(cid, IPC_RMID, NULL) < 0) printf("delete_queue");
	else printf("Queue deleted successfully.\n");
}

void login()
{
    my_msg msg;
    msg.mtype = LOGIN;
    msg.client_id = cid;
    sprintf(msg.mtext, "%s", "Login");

    if ( msgsnd(qid, &msg, MSG_SIZE, 0) < 0) error("login1");
    if ( msgrcv(cid, &msg, MSG_SIZE, 0, 0) < 0)     error("login2");
    int sid = -1;
    if (sscanf(msg.mtext, "%d", &sid) < 1) error("login3");
    if (sid < 0) error("login4");
    
    printf("New client with session id: %d.\n", sid);
}

int main(int argc, char ** argv)
{
    atexit(delete_queue);
	signal(SIGINT, handler);

  	char *path = getenv("HOME"); if (!path) error("getenv");
  	
	key_t key = ftok(path, PROJECT_ID); if (key == -1) error("ftok");
    qid = msgget(key, 0);  if (qid < 0) error("msgget");
    
    key_t ckey = ftok(path, getpid()); if (ckey == -1) error("ftok2");
    cid = msgget(ckey, IPC_CREAT | IPC_EXCL | 0666); if (cid < 0) error("msgget2");

    login();

    char * buff; size_t len = 0;
    char delims[3]= {' ', '\n', '\t'};
    while(1)
    {
        printf(">>");
        getline(&buff, &len, stdin);
        char * rest;
        char * cmd = strtok_r(buff,delims, &rest); if (!cmd){ printf("empty string\n"); continue; }
        rest = strtok_r(rest, "\n", &rest);
        proceed(cmd, rest);
    }

    return 0;
}
