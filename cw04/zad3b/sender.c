#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

int sig_cnt = 0;

void error(char *msg) { printf("%s", msg); exit(0); }

void wait_for_signal()
{
	sigset_t mask; sigemptyset(&mask);
	sigsuspend(&mask);
}

void block_signals()
{
	sigset_t mask;
    sigfillset(&mask);
	sigprocmask(SIG_SETMASK, &mask, NULL); 
}

//----- HANDLERS -----
void count_handler()
{
	sig_cnt++;
}

void end_handler(int sig_no, siginfo_t * info, void * context) 
{
	block_signals();

	if(info->si_value.sival_int!=0)  printf("[MESSAGE FROM CATCHER] %d signals was received successfully.\n",info->si_value.sival_int); //if sigqueue
}

//----- SIGACTION -----
void create_sigaction(int sig_no, void (*handler)(int, siginfo_t*, void*))
{
    static struct sigaction act;
    act.sa_sigaction = handler;
    act.sa_flags = SA_SIGINFO;
    sigfillset(&act.sa_mask); sigdelset(&act.sa_mask, sig_no);
    sigaction(sig_no,&act, NULL);
}

//----- MAIN -----
int main(int argc, char ** argv)
{
    if (argc != 4) error("Wrong no arguments");
    
    pid_t catcher_pid = atoi(argv[1]);
    int no_signals = atoi(argv[2]);
    char * mode = argv[3];

    if (strcmp(mode, "KILL") == 0)
    {
    	create_sigaction(SIGUSR1,count_handler);
        create_sigaction(SIGUSR2,end_handler);
        
        for (int i = 0; i < no_signals; i++)
        {
        	kill(catcher_pid, SIGUSR1);
        	wait_for_signal();
        }
		sleep(1);
		kill(catcher_pid, SIGUSR2);
        wait_for_signal();
    }
    else if (strcmp(mode, "SIGQUEUE") == 0)
    {
    	create_sigaction(SIGUSR1,count_handler);
        create_sigaction(SIGUSR2,end_handler);
        
        for (int i = 0; i < no_signals; i++) 
        {
        	kill(catcher_pid, SIGUSR1);
        	wait_for_signal();
        }
		sleep(1);
		kill(catcher_pid, SIGUSR2);
        wait_for_signal();
    }
    else if (strcmp(mode, "SIGRT") == 0)
    {
    	create_sigaction(SIGRTMIN,count_handler);
        create_sigaction(SIGRTMIN+1,end_handler);
        
        for (int i = 0; i < no_signals; i++) 
        {
        	kill(catcher_pid, SIGRTMIN);
        	wait_for_signal();
        }
		sleep(1);
		kill(catcher_pid, SIGRTMIN+1);
		wait_for_signal();
    }
    else error("Wrong argument");

    printf("Sent %d signals to %d and received %d.\n", no_signals, catcher_pid, sig_cnt);

    return 0;
}
