#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <ctype.h>//isspace()
#include <unistd.h>//write read...
#include <fcntl.h>//o_create itp..

void error(char * msg){ printf("%s\n", msg);exit(0);}

void libCopy(char *filename1,char *filename2)
{
	FILE *file1,*file2;
    if (!(file1 = fopen(filename1, "r"))) error("First file does not exist");
    file2 = fopen(filename2, "w");
    
    char c;
    int emptyLine=1;
    while ((c = fgetc(file1)) != EOF) 
    {
		if(c=='\n') 
		{
			if(emptyLine==0) fwrite(&c,sizeof( char ),1,file2);
			emptyLine=1;
		}
		else if(isspace(c)) 
		{
			if(emptyLine==0) fwrite(&c,sizeof( char ),1,file2);
		}
		else
		{
			fwrite(&c,sizeof( char ),1,file2);
			emptyLine=0;
		}
	}
	fclose(file1);
	fclose(file2);
}

void sysCopy(char *filename1,char *filename2)
{
	int file1=open(filename1, O_RDONLY); if(file1<0) error("First file does not exist");
	int file2=open(filename2,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);

	char c;
	int emptyLine=1;
	while(read(file1,&c,1)==1) 
	{
		if(c=='\n') 
		{
			if(emptyLine==0) write(file2,&c,1);
			emptyLine=1;
		}
		else if(isspace(c)) 
		{
			if(emptyLine==0) write(file2,&c,1); 
		}
		else
		{
			write(file2,&c,1); 
			emptyLine=0;
		}
	}
	close(file1);
	close(file2);
}

int main(int argc, char** argv)
{
	char filename1[30];
	char filename2[30];
	if(argc==1)
	{
		printf("Type first file name: ");
		scanf("%s",filename1);
		printf("Type second file name: ");
		scanf("%s",filename2);
	}
	else if(argc==3)
	{
		strcpy(filename1,argv[1]);
		strcpy(filename2,argv[2]);
	}
	else
	{
		error("Wrong arguments");
	}
	
	FILE *raportFile = fopen("./pomiar_zad_1.txt", "a");
	
	FILE *file1;
	if (!(file1 = fopen(filename1, "r"))) error("First file does not exist");
    fseek(file1, 0L, SEEK_END);
	printf("Size of file: %ld\n",ftell(file1));
	fprintf(raportFile, "Size of file: %ld\n",ftell(file1));
	fclose(file1);
	
	clock_t start = clock();
	libCopy(filename1,filename2);
	clock_t delta = clock() - start;
	printf("Lib time: %f\n", (double)delta/ CLOCKS_PER_SEC * 1000);
	fprintf(raportFile, "Lib time: %f\n", (double)delta/ CLOCKS_PER_SEC * 1000);
	
	start = clock();
	sysCopy(filename1,filename2);
	delta = clock() - start;
	printf("Sys time: %f\n", (double)delta/ CLOCKS_PER_SEC * 1000);
	fprintf(raportFile, "Sys time: %f\n\n", (double)delta/ CLOCKS_PER_SEC * 1000);

	fclose(raportFile);
	return 0;
}

