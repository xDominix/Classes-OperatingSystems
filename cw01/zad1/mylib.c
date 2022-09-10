#include <ctype.h>
#include "mylib.h"

static char template[] = "/tmp/myfileXXXXXX";

int isNumber(char* s) { for (int i = 0; i < strlen(s); i++) {if (!isdigit(s[i])) return 0;} return 1;}

struct BlockArray createBlockArray(int size)
{
    struct BlockArray array;

    array.blocks = (struct Block*) calloc(size, sizeof(struct Block));
    array.index = -1;

    return array;
};

struct Block wcFile(char *file)
{
    char tempFile[100];
	strcpy(tempFile, template);
	int fd = mkstemp(tempFile);

    char command[50];
    strcpy(command, "wc ");
    strcat(command, file);
    strcat(command, " > ");
    strcat(command, tempFile);

    system(command);

    FILE *f = fopen(tempFile, "r");

    fseek(f, 0, SEEK_END);
    long fileSize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char * result = (char*) calloc((size_t) fileSize, sizeof(char));
    int i=0;
    char ch;
    while(!feof(f)) if((ch = (char) fgetc(f)) != -1) result[i++] = ch;
    result[i] = '\0';

    struct Block newBlock;
    char * token = strtok(result, " ");

    if (!isNumber(token)){perror("Command wrong reading [0]");exit(1);}
    newBlock.l = atoi(token);
    token = strtok(NULL, " ");
     if (!isNumber(token)){perror("Command wrong reading [1]");exit(1);}
    newBlock.w = atoi(token);
    token = strtok(NULL, " ");
     if (!isNumber(token)){perror("Command wrong reading [2]");exit(1);}
    newBlock.c = atoi(token);
    token = strtok(NULL, " ");
    newBlock.file = (char*) calloc(strlen(token),sizeof(char));
    strcpy(newBlock.file,token);

    free(result);
    fclose(f);
    close(fd);
	unlink(tempFile);		

    return newBlock;
}

int addBlock(struct BlockArray blockArray, struct Block newBlock){
    blockArray.blocks[blockArray.index+1] = newBlock;
    return blockArray.index+1;
};

void removeBlock(struct BlockArray blockArray, int index){
    free(blockArray.blocks[index].file);
};