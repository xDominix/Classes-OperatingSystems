#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

int sig_cnt = 0;

void error(char *msg) { printf("%s", msg); exit(0); }

void block_signals()
{
	sigset_t mask; sigfillset(&mask);
	sigprocmask(SIG_SETMASK, &mask, NULL); 
}

//----- HANDLERS -----
void count_handler(int sig_no, siginfo_t * info, void * context)
{
	sig_cnt++;
    
    kill(info->si_pid, SIGUSR1);
    
    sigset_t mask; sigfillset(&mask); sigdelset(&mask, SIGUSR1); sigdelset(&mask, SIGUSR2); 
	sigsuspend(&mask);
}

void rt_count_handler(int sig_no, siginfo_t * info, void * context)
{
	sig_cnt++;
    
    kill(info->si_pid, SIGRTMIN);
    
    sigset_t mask; sigfillset(&mask); sigdelset(&mask, SIGRTMIN); sigdelset(&mask, SIGRTMIN+1); 
	sigsuspend(&mask);
}

void end_handler(int sig_no, siginfo_t * info, void * context) 
{
	block_signals();
	
	kill(info->si_pid, sig_no);
	
	printf("Received %d signals from %d.\n", sig_cnt, info->si_pid);
}

void queue_end_handler(int sig_no, siginfo_t * info, void * context)
{
	block_signals();
	
	sigqueue(info->si_pid, SIGUSR2,(union sigval){sig_cnt});//info how many signals catcher received
	
	printf("Received %d signals from %d.\n", sig_cnt, info->si_pid);
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
    if (argc != 2) error("Wrong no arguments");
    char *mode = argv[1];
	
	printf("PID: %d\n", getpid());
	
    if (strcmp(mode, "KILL") == 0)
    {
        create_sigaction(SIGUSR1,count_handler);
        create_sigaction(SIGUSR2,end_handler);
        
       	sigset_t mask; sigfillset(&mask); sigdelset(&mask, SIGUSR1); sigdelset(&mask, SIGUSR2); sigsuspend(&mask); //waiting for usr signal
    }
    else if (strcmp(mode, "SIGQUEUE") == 0)
    {
        create_sigaction(SIGUSR1,count_handler);
        create_sigaction(SIGUSR2,queue_end_handler);
        
       	sigset_t mask; sigfillset(&mask); sigdelset(&mask, SIGUSR1); sigdelset(&mask, SIGUSR2); sigsuspend(&mask); //waiting for usr signal
    }
    else if (strcmp(mode, "SIGRT") == 0)
    {
        create_sigaction(SIGRTMIN,rt_count_handler);
        create_sigaction(SIGRTMIN+1,end_handler);
        
       	sigset_t mask; sigfillset(&mask); sigdelset(&mask, SIGRTMIN); sigdelset(&mask, SIGRTMIN+1); sigsuspend(&mask);// waiting for rt signal
    }
    else error("Wrong argument");

    return 0;
}
