#ifndef commands_h
#define commands_h

#define MAX_CLIENTS  10
#define PROJECT_ID 'x'

int error(char * msg){ printf("error: ");printf(msg);printf("\n"); exit(0);}

typedef enum mtype { LOGIN = 1, STOP = 2, TOALL = 3, TOONE = 4,LIST = 5} mtype;

typedef struct my_msg
{
 	int client_id; //from who
 	int to_id; //to whom (optional)
    long mtype;
    char mtext[1000];
} my_msg;

const size_t MSG_SIZE = sizeof(my_msg) - sizeof(long);

#endif
