#ifndef MYLIB_H
#define MYLIB_H
#include <stdio.h>
#include <wait.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>

#include <sys/sem.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>

const key_t semKey = 0xaaaaaaaa;
const key_t shmKey = 0xaaaaaaab;

const size_t OVEN_ID = 0;
const size_t TABLE_ID = 1;

typedef struct
{
	int spot[5];
    int next;//next index
} Oven;

typedef struct
{
    int spot[5];
    int next;//next index
} Table;

union semun {   int val;  struct semid_ds *buf;    unsigned short  *array;   struct seminfo  *__buf;  }; //linux

unsigned long getTimestamp()
{
    struct timespec spec; clock_gettime(1, &spec);
    return spec.tv_sec*1000 + spec.tv_nsec / 1000000;
}
#endif
