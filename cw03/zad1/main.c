#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

void error(char *msg) { printf("%s", msg); exit(0); }

int main(int argc, char **argv)
{
	if(argc != 2) error("Wrong no arguments");
	
	int noChildren =  atoi(argv[1]);
	
	if(noChildren<=0) return 0;
	
	pid_t pid;
    do
    {
        pid = fork();
        noChildren--;
    }
    while(pid && noChildren);//pid!=0 to proces rodzica i on ma wykonywac petle

    if (pid == 0) printf("Proces dziecka (PID: %d)\n", getpid());
    else 
    {
    	wait(NULL);
    	//printf("Proces rodzica (PID: %d)\n",getpid());
    }
    
	return 0;
}
