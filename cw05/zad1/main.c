#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_CMDS 4

void error(char *msg) { printf("%s", msg); exit(0); }

char** split(char * cmd)
{
    char ** argv = NULL; int count = 0;
    cmd = strtok(cmd, " \n");
    while (cmd)
    {
        count++;
        argv = realloc(argv, count * sizeof(char*));
        argv[count - 1] = cmd;
        cmd = strtok(NULL,  " \n");
    }
    argv = realloc(argv, (count + 1) * sizeof(char*));
    argv[count] = NULL;

    return argv;
}

int run(char * line, ssize_t ssize)
{
    int pipes[2][2];

    char * cmds[MAX_CMDS];
	int size = 0;
	
    line = strtok(line, "|");
    while ( line ){ cmds[size++] = line; line = strtok(NULL, "|"); }
	
	int i;
    for (i = 0; i < size; ++i)
    {
        if (i != 0)
        {
            close(pipes[i % 2][0]);
            close(pipes[i % 2][1]);
        }

        pipe(pipes[i % 2]);

        if (vfork() == 0)
        {
        	if (i != 0) 
            {
                close(pipes[(i+1) % 2][1]);
             	dup2(pipes[(i+1) % 2][0], STDIN_FILENO);
            }
            
            if (i != size - 1)
            {
                close(pipes[i % 2][0]);
                dup2(pipes[i % 2][1], STDOUT_FILENO);
            }

            char ** argv = split(cmds[i]);
         	execvp(argv[0], argv);
        }
    }
    close(pipes[i % 2][0]);
    close(pipes[i % 2][1]);
    wait(NULL);
}

int main(int argc, char ** argv)
{
    if (argc  != 2) error("Wrong no arguments");

    FILE * fp = fopen(argv[1], "r");
    if (fp == NULL) error("Open file error");

	char * buffer=NULL; size_t size = 0; ssize_t nread;
    while ((nread = getline(&buffer, &size, fp)) != -1) 
    {
        if (vfork()== 0)
        {
            run(buffer, nread);
            exit(0);
        }
        else wait(NULL);
    }

 	if (buffer!=NULL) free(buffer);
    fclose(fp);
   
    return 0;
}
