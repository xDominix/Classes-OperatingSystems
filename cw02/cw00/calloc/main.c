#include <stdlib.h>

int main()
{
int *p1;
char *p2;
p1 = (int*) malloc(10*sizeof(int));
p2 = (char*) calloc(100,sizeof(char));

p1[0]=10;
p2[1]='a';
printArray(p1,10);
printArrayChar(p2,100);

free(p1);
free(p2);
return 0;
}
