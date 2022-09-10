#include <stdlib.h>
#include "mylib.h"
#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <sys/times.h>
#include <dlfcn.h>
#include <ctype.h>
#include <time.h>

void error(char * msg){ printf("%s", msg); exit(0); }

int main(int argc,char **argv)
{
    struct Block temp;
    struct BlockArray blockArray;
    int index;
    for(int i=1; i<argc; ++i){

        if(strcmp(argv[i], "create_table") == 0)
        {
            if(!isNumber(argv[i+1])) error("Error during create_table");

            printf("Create table size %s\n", argv[i+1]);

            blockArray = createBlockArray(atoi(argv[i+1]));
            i += 1;
        }
        else if(strcmp(argv[i], "wc_files") == 0)
        {
            if(!isNumber(argv[i+1])) error("Invalid wc_files [count] [.txt]... format");
            int filec = atoi(argv[i+1]);
            if(filec < 1) error("Invalid file count");

            printf("Wc files\n");

            for(int j=0;j<filec;j++)
            {
                temp = wcFile(argv[i+2+j]);
                index = addBlock(blockArray, temp);
                blockArray.index = index;
                printf("Wc %s: %d %d %d \n",argv[i+2+j],temp.l,temp.w,temp.c);
            };

            i += filec+1;
        }
        else if(strcmp(argv[i], "remove_block") == 0)
        {
            if(!isNumber(argv[i+1])) error("Error during remove_block");

            printf("Remove block %s\n", argv[i+1]);

            removeBlock(blockArray, atoi(argv[i+1]));
            i += 1;
        }
        else error("Wrong argument");
    }

    return 0;
}