#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

bool santaSleep=true;
pthread_cond_t santaSleepCond=PTHREAD_COND_INITIALIZER;
pthread_mutex_t santaSleepMutex=PTHREAD_MUTEX_INITIALIZER;

int waitReindeers=0;
pthread_mutex_t waitReindeersMutex=PTHREAD_MUTEX_INITIALIZER;
int busyReindeers[9]={false};
pthread_cond_t busyReindeersCond= PTHREAD_COND_INITIALIZER;
pthread_mutex_t busyReindeersMutex=PTHREAD_MUTEX_INITIALIZER;

int waitElves[3]={-1,-1,-1};
pthread_mutex_t waitElvesMutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t waitElvesCond= PTHREAD_COND_INITIALIZER;
int busyElves[10]={false};
pthread_cond_t busyElvesCond= PTHREAD_COND_INITIALIZER;
pthread_mutex_t busyElvesMutex=PTHREAD_MUTEX_INITIALIZER;

void * santaTask(void * data)
{
    while(true)
    {
        pthread_mutex_lock(&santaSleepMutex);
        while(santaSleep) pthread_cond_wait(&santaSleepCond, &santaSleepMutex);
        puts("Mikołaj: budzę się");
        santaSleep = true;
        pthread_mutex_unlock(&santaSleepMutex);

        bool shouldHelpReindeers = false;
        pthread_mutex_lock(&waitReindeersMutex);
        if(waitReindeers == 9)
        {
            waitReindeers = 0;
            pthread_mutex_lock(&busyReindeersMutex);
            for(int i = 0; i < 9; i++) busyReindeers[i] = true;
            pthread_mutex_unlock(&busyReindeersMutex);
            pthread_cond_broadcast(&busyReindeersCond);
            shouldHelpReindeers = true;
        }
        pthread_mutex_unlock(&waitReindeersMutex);
        if(shouldHelpReindeers)
        {
            puts("Mikołaj: dostarczam zabawki");
            usleep((rand() % 2000 + 2000)*1000);
        }

 		bool shouldHelpElves = true;
        pthread_mutex_lock(&waitElvesMutex);
        for(int i = 0; i < 3; i++)  if(waitElves[i] == -1) shouldHelpElves = false; 
        pthread_mutex_unlock(&waitElvesMutex);
        if(shouldHelpElves)
        {
        	printf("Mikołaj: rozwiązuję problemy elfów %d %d %d\n", waitElves[0], waitElves[1],  waitElves[2]);

            pthread_mutex_lock(&busyElvesMutex);
            pthread_mutex_lock(&waitElvesMutex);
            
            for(int i = 0; i < 3; i++)  busyElves[waitElves[i]] = true;
            pthread_cond_broadcast(&busyElvesCond);
            pthread_mutex_unlock(&busyElvesMutex);
            
            usleep((rand() % 1000 + 1000)*1000);
            
            for(int i = 0; i < 3; i++) waitElves[i] = -1;
            pthread_cond_broadcast(&waitElvesCond);
            pthread_mutex_unlock(&waitElvesMutex);
        }

        puts("Mikołaj: zasypiam");
    }
}

void * reindeerTask(void * data)
{
    int myID = -1;
    while(true)
    {
        usleep((rand() % 5000 + 5000)*1000);

        pthread_mutex_lock(&waitReindeersMutex);
        if(myID == -1) myID = waitReindeers; 
        waitReindeers++;
        printf("Renifer: czeka %d reniferów na Mikołaja, %d\n", waitReindeers, myID);
        if(waitReindeers == 9)
        {
            printf("Renifer: wybudzam Mikołaja, %d\n", myID);
            pthread_mutex_lock(&santaSleepMutex);
            santaSleep = false;
            pthread_cond_signal(&santaSleepCond);
            pthread_mutex_unlock(&santaSleepMutex);
        }
        pthread_mutex_unlock(&waitReindeersMutex);

        usleep((rand() % 2000 + 2000)*1000);

        pthread_mutex_lock(&busyReindeersMutex);
        while(busyReindeers[myID] == false) pthread_cond_wait(&busyReindeersCond, &busyReindeersMutex);
        busyReindeers[myID] = false;
        pthread_mutex_unlock(&busyReindeersMutex);
    }
}

void * elfTask()
{
    int myID;
    pthread_mutex_lock(&busyElvesMutex);
    for(int i = 0; i < 10; i++)
    {
        if(busyElves[i] == false)
        {
            myID = i;
            busyElves[i] = true;
            break;
        }
    }
    if(myID == 9)
    {
        for(int i = 0; i < 10; i++) busyElves[i] = false;
        pthread_cond_broadcast(&busyElvesCond);
    }
    pthread_mutex_unlock(&busyElvesMutex);

    pthread_mutex_lock(&busyElvesMutex);
    while(busyElves[myID] == true) pthread_cond_wait(&busyElvesCond, &busyElvesMutex);
    pthread_mutex_unlock(&busyElvesMutex);

    while(true)
    {
        usleep((rand() % 3000 + 2000)*1000);
        bool takenPosition = false;
        pthread_mutex_lock(&waitElvesMutex);
        while(!takenPosition)
        {
            for(int i = 0; i < 3; i++)
            {
                if(waitElves[i] == -1)
                {
                    waitElves[i] = myID;
                    takenPosition = true;
                    printf("Elf: czeka %d elfów na mikołaja, %d\n", i+1, myID);
                    if(i == 2)
                    {
                        printf("Elf: wybudzam Mikołaja, %d\n", myID);
                        pthread_mutex_lock(&santaSleepMutex);
                        santaSleep = false;
                        pthread_cond_signal(&santaSleepCond);
                         pthread_mutex_unlock(&santaSleepMutex);
                    }
                    break;
                }
            }
            if(!takenPosition)
            {
                printf("Elf: czeka na powrót elfów, %d\n", myID);
                pthread_cond_wait(&waitElvesCond, &waitElvesMutex);
            }
        }
        pthread_mutex_unlock(&waitElvesMutex);    

        pthread_mutex_lock(&busyElvesMutex);
        while(!busyElves[myID]) pthread_cond_wait(&busyElvesCond, &busyElvesMutex);
        busyElves[myID] = false;
        pthread_mutex_unlock(&busyElvesMutex);  
        usleep((rand() % 1000 + 1000)*1000);
    }
}

int main()
{
    srand(time(NULL));

    pthread_t santa;
    pthread_t reindeers[9];
    pthread_t elves[10];

    pthread_create(&santa, NULL, santaTask, NULL);
    for(int i = 0; i < 9; i++) pthread_create(reindeers+i, NULL, reindeerTask, NULL);
    for(int i = 0; i < 10; i++) pthread_create(elves+i, NULL, elfTask, NULL);

    pthread_join(santa, NULL);
    for(int i = 0; i < 9; i++) pthread_join(reindeers[i], NULL);
    for(int i = 0; i < 10; i++) pthread_join(elves[i], NULL);
    
    return 0;
}
