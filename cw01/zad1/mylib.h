#ifndef mylib_h
#define mylib_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct BlockArray{
    int index;
    struct Block *blocks;
};

struct Block{
    char *file;
    int l;
    int w;
    int c;
};

struct BlockArray createBlockArray(int size);
struct Block wcFile(char *file);
int addBlock(struct BlockArray blockArray, struct Block newBlock);
void removeBlock(struct BlockArray blockArray, int index);

int isNumber(char* s);

#endif //mylib_h