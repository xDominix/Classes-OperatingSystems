#include "mylib.h"

pid_t * children = NULL;   
size_t childCnt = 0;

int semID = -1;

Oven * oven;//shared oven
Table * table;//shared table

void error(char *msg) { printf("%s\n", msg); exit(0); }

void handler(){ for(size_t i = 0; i < childCnt; i++){ kill(children[i], SIGINT); } } //parent handler
void chandler(){ shmdt(oven); shmdt(table); exit(0); } //child handler

void cookLoop()
{
	signal(SIGINT, chandler);
	srand(time(NULL));
	 
    pid_t myPid = getpid();
    while(1)
    {
        int pizzaType = rand() % 10;
      	printf("(%d %lu) %d --->       --->       --->\n", myPid, getTimestamp(), pizzaType);
      	
        usleep((1000+ rand() % 1000 )* 1000);//przygotowywanie
       
        int atPos = 0,inserted = 0;
        struct sembuf sops = {.sem_num = OVEN_ID, .sem_flg = 0};
        while(inserted==0)
        {
        	sops.sem_op = -1;
        	semop(semID, &sops, 1);
            
            if(oven->spot[oven->next] == -1) //jesli jest wolny piekarnik
            {
                oven->spot[oven->next] = pizzaType;
                atPos = oven->next;
                oven->next = (oven->next+1) % 5;
                inserted = 1;
                
                int ovenCnt = 0;
   				for(size_t i = 0; i < 5; i++) if(oven->spot[i] != -1) { ovenCnt++;  }
        		printf("(%d %lu)   -%d->   %d   --->       --->\n", myPid, getTimestamp(), pizzaType, ovenCnt);
            }
            
            sops.sem_op = 1;
            semop(semID, &sops, 1);            
        }
        
        usleep((4000+rand() % 1000) * 1000); //pieczenie
        
        inserted = 0;
        while(inserted==0)
        {
        	sops.sem_num = TABLE_ID;
            sops.sem_op = -1;
            semop(semID, &sops, 1);
            
            if(table->spot[table->next] == -1)//jesli jest wolny stol
            {
                sops.sem_num = OVEN_ID;
                semop(semID, &sops, 1);
                oven->spot[atPos] = -1;
                sops.sem_op = 1;
                
                semop(semID, &sops, 1);
                inserted = 1;
                
                table->spot[table->next] = pizzaType;
                table->next = (table->next + 1) % 5;
                
				int ovenCnt = 0;
			 	for(size_t i = 0; i < 5; i++) if(oven->spot[i] != -1) ovenCnt++;
			 	int tableCnt= 0;
			 	 for(size_t i = 0; i < 5; i++) if(table->spot[i] != -1) tableCnt++;
				printf("(%d %lu)   --->   %d   -%d->   %d   --->\n", myPid, getTimestamp(), ovenCnt ,pizzaType, tableCnt);
            }
            sops.sem_num = TABLE_ID;
            sops.sem_op = 1;
            semop(semID, &sops, 1);            
        }
    }
}

void driverLoop()
{
	signal(SIGINT, chandler);
	srand(time(NULL));
	
	struct sembuf sops = {.sem_num = TABLE_ID, .sem_flg = 0};
    pid_t myPid = getpid();
    while(1)
    {
        sops.sem_op = -1;
        semop(semID, &sops, 1);
        
        int toTake = -1, pizzaType = -1;
        for(int i = 0; i < 5; i++) if(table->spot[i] != -1) { toTake = i; }
      
        if(toTake != -1) //jesli jest cos do zabrania
        {
        	int tableCnt = 0;
        	for(int i = 0; i < 5; i++) if(table->spot[i] != -1) { tableCnt++; }
            printf("(%d %lu)   --->       --->   %d   -%d->\n", myPid, getTimestamp(), tableCnt-1,table->spot[toTake]);
            pizzaType = table->spot[toTake];
            table->spot[toTake] = -1;
        }
        
        sops.sem_op = 1;
        semop(semID, &sops, 1);
        
         if(toTake != -1) //jesli bylo zabrane
         {
            usleep((1000+ rand() % 4000 )* 1000); //dostawa
            
            printf("(%d %lu)   --->       --->       ---> %d\n", myPid, getTimestamp(),pizzaType);
            
            usleep((1000+ rand() % 4000 )* 1000); //powrot
        }
    }
}

int main(int argc, char ** argv)
{
    if(argc < 3) error("Wrong arguments");
    
    int N = atoi(argv[1]), M = atoi(argv[2]);
    if(N < 1 || M < 1) error("Invalid arguments");
    
    signal(SIGINT, handler);
    printf("%s\n","( PID TIME ) PREPARING TYPE -TYPE->   OVEN COUNT  -TYPE->   TABLE COUNT   -TYPE-> DELIVERED TYPE");

    semID = semget(semKey, 2, IPC_CREAT | IPC_EXCL | 0666);
    if(semID == -1) error("semget");
    
    int shmID = shmget(shmKey, sizeof(Oven) + sizeof(Table), IPC_CREAT | IPC_EXCL | 0666);
    if(shmID == -1) error("shmget");
    oven = shmat(shmID, NULL, 0);
    table = (Table*)((void*)oven + sizeof(Oven));
    for(size_t i = 0; i < 5; i++) { oven->spot[i] = -1;  table->spot[i] = -1; }
    oven->next = 0; table->next = 0;

    union semun arg; arg.val = 1;
    semctl(semID, OVEN_ID, SETVAL, arg);
    semctl(semID, TABLE_ID, SETVAL, arg);

    children = malloc(sizeof(pid_t)*(N+M));
    pid_t chPid = 0;
    for(int i = 0; i < N; i++)
    {
        if((chPid = fork()) == 0)
        {
            cookLoop();
            return 0;
        }
        else children[childCnt++] = chPid;
    }

    for(int i = 0; i < M; i++){
        if((chPid = fork()) == 0)
        {
            driverLoop();
            return 0;
        }
        else children[childCnt++] = chPid;
    }

    wait(NULL);
    
    free(children);
    semctl(semID, 0, IPC_RMID);
    shmdt(oven);
    shmctl(shmID, IPC_RMID, NULL);
    return 0;
}
