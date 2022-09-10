#include<stdio.h>	


void printArray(int *a, int len) {
    for (int i = 0; i < len; i++) printf("%d ", a[i]);
}
void printArrayChar(char *a, int len) {
    for (int i = 0; i < len; i++) printf("%c ", a[i]);
}
