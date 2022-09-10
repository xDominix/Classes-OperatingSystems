#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //fork execl
#include <dirent.h>
#include <wait.h>

void error(char *msg) { printf("%s", msg); exit(0); }

int isPatternInFile(char * path, char * pattern)
{
	FILE* file = fopen(path, "r");
	char buf[255];
	while(fgets(buf, 255, file))
	{
		if (strstr(buf, pattern) != NULL) { fclose(file); return 1; }
	}
	fclose(file);
	return 0;
}

void searchPattern(char* dirName, char* pattern, int depth)	
{
    DIR* directory = opendir(dirName);
    if (directory == NULL) error("Open directory failed");

    struct dirent* file;
    char *path = (char*) calloc(100, sizeof(char));

    while ((file = readdir(directory)) != NULL)
    {
        if (strcmp(file->d_name, ".") != 0 && strcmp(file->d_name, "..") != 0)
        {
        	strcpy(path, dirName);
        	strcat(path, "/");
        	strcat(path, file->d_name);
        	
        	if(file->d_type == DT_REG && strlen(file->d_name) > 4 && strcmp(file->d_name + strlen(file->d_name) - 4, ".txt") == 0)
		    {
		        if (isPatternInFile(path, pattern))
		        {
		            printf("Found pattern in path: %s (PID: %d)\n", path, getpid());
		        }
		    }
		    else if (file->d_type == DT_DIR)
		    {
		        pid_t pid = fork();
		        if (pid != 0) {int s = 0;wait(&s);}
		        else
		        {
		        	//printf("New pid: %d\n",getpid());
		            char depthStr[9]; sprintf(depthStr, "%d", depth-1);
		            execl("./main", "main", path, pattern,depthStr,NULL);
		        }
		    }
        }
    }
    free(path);
    closedir(directory);
}

int main(int argc, char **argv)
{
	if(argc != 4) error("Wrong no. arguments");

    char* dirName = argv[1];
    char* pattern = argv[2];
    int depth = atoi(argv[3]);
    if (depth < 0) return 0;

    searchPattern(dirName, pattern, depth);

	return 0;
}
