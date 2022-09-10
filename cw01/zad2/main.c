#include <stdlib.h>
#include "mylib.h"
#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <sys/times.h>
#include <dlfcn.h>
#include <ctype.h>
#include <time.h>

FILE * raportFile;
const char *raportName = "./raport2.txt";

void error(char * msg){ printf("%s", msg); exit(0); }

double deltaTime(clock_t t1, clock_t t2){ return ((double)(t2 - t1) / sysconf(_SC_CLK_TCK));}

void writeRaport(clock_t start, clock_t end, struct tms* t_start, struct tms* t_end){
    printf("- REAL TIME: %f\n", deltaTime(start,end));
    printf("- USER TIME: %f\n", deltaTime(t_start->tms_utime, t_end->tms_utime));
    printf("- SYSTEM TIME: %f\n", deltaTime(t_start->tms_stime, t_end->tms_stime));

    fprintf(raportFile, "- REAL TIME: %f\n", deltaTime(start, end));
    fprintf(raportFile, "- USER TIME: %f\n", deltaTime(t_start->tms_utime, t_end->tms_utime));
    fprintf(raportFile, "- SYSTEM TIME: %f\n", deltaTime(t_start->tms_stime, t_end->tms_stime));
}

int main(int argc,char **argv) {
    //time
    int size = argc;
    raportFile = fopen(raportName, "a");
    struct tms * tms[size];
    clock_t time[size];
    for(int i = 0; i < size; i++){
        tms[i] = calloc(1, sizeof(struct tms *));
        time[i] = 0;
    }

    int current = 0;//time
    struct Block temp;
    struct BlockArray blockArray;
    int index;
    for(int i=1; i<argc; ++i){

        time[current] = times(tms[current]);//time
        current ++;//time

        if(strcmp(argv[i], "create_table") == 0)
        {
            if(!isNumber(argv[i+1])) error("Error during create_table");

            printf("Create table size %s\n", argv[i+1]);
            fprintf(raportFile, "Create table size %s\n", argv[i+1]);

            blockArray = createBlockArray(atoi(argv[i+1]));
            i += 1;
        }
        else if(strcmp(argv[i], "wc_files") == 0)
        {
            if(!isNumber(argv[i+1])) error("Invalid wc_files [count] [.txt]... format");
            int filec = atoi(argv[i+1]);
            if(filec < 1) error("Invalid file count");

            printf("Wc files\n");
            fprintf(raportFile, "Wc files\n");

            for(int j=0;j<filec;j++)
            {
                temp = wcFile(argv[i+2+j]);
                index = addBlock(blockArray, temp);
                blockArray.index = index;
                printf("Wc %s: %d %d %d \n",argv[i+2+j],temp.l,temp.w,temp.c);
                fprintf(raportFile, "Wc %s: %d %d %d \n",argv[i+2+j],temp.l,temp.w,temp.c);
            };

            i += filec+1;
        }
        else if(strcmp(argv[i], "remove_block") == 0)
        {
            if(!isNumber(argv[i+1])) error("Error during remove_block");

            printf("Remove block %s\n", argv[i+1]);
            fprintf(raportFile, "Remove block %s\n", argv[i+1]);

            removeBlock(blockArray, atoi(argv[i+1]));
            i += 1;
        }
        else error("Wrong argument");
        
        //time
        time[current] = times(tms[current]);
        writeRaport(time[current-1], time[current],tms[current-1], tms[current]);
        current ++;
    }

    fclose(raportFile);
    return 0;
}