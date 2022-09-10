#define _XOPEN_SOURCE 500 // FTW_PHYS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h> //DIR
#include <sys/stat.h> //stat
#include <time.h> //strftime
#include <ftw.h> //nftw

int *typeCnt;//licznik typow (metoda readdir,opendir)
int *nftwTypeCnt;//licznik typow (metoda nftw)

void error(char *msg) { printf("%s", msg); exit(0); }

//READDIR OPENDIR METHOD
char *typeToString(unsigned char type)
{
    switch(type) 
    {
        case 8: return "file"; //DT_REG
        case 4: return "dir"; //DT_DIR
        case 2: return "char dev"; //DT_CHR
        case 6: return "block dev"; //DT_BLK
        case 1: return "fifo"; //DT_FIFO
        case 10: return "slink"; //DT_LNK
        case 12: return "sock"; //DT_SOCK
        default: return "other";
    };
}

void info(char *dirName)
{
	DIR* directory = opendir(dirName);
    if(directory==NULL) error("Open directory failed");
	
	char buffer[20];
	char *path = (char*) calloc(100, sizeof(char));
    struct dirent *file;
    struct stat stats;
    while((file = readdir(directory))!=NULL)
    {
        strcpy(path, dirName);
        strcat(path, "/");
        strcat(path, file->d_name);
		
        lstat(path, &stats);
        
		if(strcmp(file->d_name, ".") != 0 && strcmp(file->d_name, "..") != 0)//hidden dirs like '.' or '..'
		{
		    printf("\nPath: %s\nLinks: %ld\nType: %s\nSize: %ldB\n",realpath(path, NULL),stats.st_nlink,typeToString(file->d_type),stats.st_size);
		    strftime(buffer, 20, "%d.%m.%Y %H:%M:%S", localtime(&stats.st_atime));
		    printf("Last access: %s\n", buffer);
		    strftime(buffer, 20, "%d.%m.%Y %H:%M:%S", localtime(&stats.st_mtime));
		   	printf("Last modification: %s\n", buffer);
		    typeCnt[file->d_type]++;
		    
		    if(strcmp(typeToString(file->d_type),"dir") == 0) info(path);
        }
    }

    free(path);
    free(file);
    closedir(directory);
}

//NFTW METHOD
char *flagToString(int flag)
{
    switch(flag) 
    {
        case FTW_F: return "file";
        case FTW_D: return "dir";
        case FTW_SL: return "slink";
        default: return "other";
    };
}

int nftw_display(const char *path, const struct stat *stats, int flagType)
{
    char buffer[20];
      
    printf("\nPath: %s\nLinks: %ld\nType: %s\nSize: %ldB\n",realpath(path, NULL),stats->st_nlink,flagToString(flagType),stats->st_size);
    strftime(buffer, 20, "%d.%m.%Y %H:%M:%S", localtime(&stats->st_atime));
    printf("Last access: %s\n", buffer);
    strftime(buffer, 20, "%d.%m.%Y %H:%M:%S", localtime(&stats->st_mtime));
   	printf("Last modification: %s\n", buffer);
    nftwTypeCnt[flagType]++;
    
    return 0;
}

void nftw_info(char *dirName)
{
    if(nftw(dirName, nftw_display, 20, FTW_PHYS)==-1) error("Nftw error");
}

//MAIN
int main(int argc, char **argv)
{
 	if(argc != 2)error("Wrong arguments");
 	char *dirName = argv[1];
   	
   	typeCnt= (int*) calloc(13, sizeof(int));
   	nftwTypeCnt= (int*) calloc(5, sizeof(int));
   	
   	printf("\n----------USING-READDIR-OPENDIR-----------\n");
   	info(dirName);
   	printf("\n--------------------USING-NFTW-------------------\n");
   	nftw_info(dirName);
   	
   	printf("\nFound: file(%d), dir(%d),char dev(%d), block dev(%d), fifo(%d), slink(%d), sock(%d)\n",
   	typeCnt[8], typeCnt[4],typeCnt[2],typeCnt[6], typeCnt[1],typeCnt[10], typeCnt[12]);
   	printf("[NFTW] Found: file(%d), dir(%d+1), slink(%d)\n",
   	nftwTypeCnt[FTW_F],nftwTypeCnt[FTW_D]-1,nftwTypeCnt[FTW_SL]);
   	
   	free(typeCnt);
   	free(nftwTypeCnt);
   	return 0;
}
