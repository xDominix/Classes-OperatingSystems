#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

void error(char * msg){ printf("%s\n", msg);exit(0);}

void libCount(char *filename,char x)
{
	FILE *file;
    if (!(file = fopen(filename, "r"))) error("File does not exist");
    
    char c;
    int charcnt=0,linecnt=0,newLine =1;
    while ((c = fgetc(file)) != EOF) {
		if(c==x)
		{
			if(newLine==1) linecnt++;
			charcnt++;
			newLine=0;
		}
		else if(c=='\n') newLine=1;
	}
	
	printf("%d %d\n",charcnt,linecnt);
	fclose(file);
}

void sysCount(char *filename,char x)
{
	int file = open(filename, O_RDONLY);
	if(file<0) error("File does not exist");
	
	char c;
	int charcnt=0,linecnt=0,newLine =1;
	while (read(file,&c,1)==1) {
		if(c==x)
		{
			if(newLine==1) linecnt++;
			charcnt++;
			newLine=0;
		}
		else if(c=='\n')newLine=1;
	}
	
	printf("%d %d\n",charcnt,linecnt);
	close(file);
}

int main(int argc, char** argv)
{
	if (argc != 3)error("Wrong argumnets");
	if (strlen(argv[1]) != 1) error("Wrong argument");

	FILE *raportFile = fopen("./pomiar_zad_2.txt", "a");
	
	FILE *file;
	if (!(file = fopen(argv[2], "r"))) error("File does not exist");
    fseek(file, 0L, SEEK_END);
	printf("Size of file: %ld\n",ftell(file));
	fprintf(raportFile, "Size of file: %ld\n",ftell(file));
	fclose(file);
	
	clock_t start = clock();
	libCount(argv[2],argv[1][0]);
	clock_t delta = clock() - start;
	printf("Lib time: %f\n", (double)delta/ CLOCKS_PER_SEC * 1000);
	fprintf(raportFile, "Lib time: %f\n", (double)delta/ CLOCKS_PER_SEC * 1000);
	
	start = clock();
	sysCount(argv[2],argv[1][0]);
	delta = clock() - start;
	printf("Sys time: %f\n", (double)delta/ CLOCKS_PER_SEC * 1000);
	fprintf(raportFile, "Sys time: %f\n\n", (double)delta/ CLOCKS_PER_SEC * 1000);

	fclose(raportFile);
    return 0;
}

