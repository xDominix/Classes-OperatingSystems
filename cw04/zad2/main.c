#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>

/* ABOUT SIGINFO_T
[siginfo_t fields] -> [function that access it]
int si_signo; -> siginfo_handler1
int si_code; //signal code
union sigval si_value; -> siginfo_handler1
int si_errno;  //If non-zero, an errno value associated with this signal.
pid_t si_pid; -> siginfo_handler1
uid_t si_uid;-> siginfo_handler1 //Real user ID of sending process.
void *si_addr; -> siginfo_handler1 //Address at which fault occurred.
int si_status;-> siginfo_handler2
int si_band; //Band event for SIGPOLL/SIGIO.
*/
void siginfo_handler1(int sig_no, siginfo_t* info, void* ucontext)
{
    printf("Test 1 (sigusr1): \n");
    printf("- Signal number: %d\n", info->si_signo);
    printf("- Signal custom value: %d\n", info->si_value.sival_int);
    printf("- Pid: %d\n", info->si_pid);
    printf("- Uid: %d\n", info->si_uid);
    printf("- Fault address: %p\n",info->si_addr);
}

void siginfo_handler2(int sig_no, siginfo_t* info, void* ucontext)
{
	printf("Test 2 (sigchld): \n");
	printf("- Signal number: %d\n",sig_no);//alternative way
	printf("- Child pid: %d\n", info->si_pid);
	printf("- Exit status: %d\n", info->si_status);
	printf("- Exit mess: %s\n", strerror( info->si_errno));
}

void nocldstop_handler(int sig_no, siginfo_t* info, void* ucontext)
 { 
	printf("Signal number: %d was sent\n",sig_no); 
}

void resethand_handler()
{
    printf("Hello from handler\n");
}

void create_sigaction(int sig_no, int flag, void (*handler)(int, siginfo_t*, void*))
{
    static struct sigaction act;
    act.sa_sigaction = handler;
    act.sa_flags = flag;
    sigemptyset(&act.sa_mask);
    sigaction(sig_no,&act, NULL);
}

int main(int argc, char** argv)
{
	//----1
    printf("\n[SA_SIGINFO] (we should find out some infos about signal)\n");
    
    create_sigaction(SIGUSR1, SA_SIGINFO, siginfo_handler1);
    create_sigaction(SIGCHLD, SA_SIGINFO, siginfo_handler2);
    
   	sigqueue(getpid(), SIGUSR1, (union sigval){47});
    if (fork() == 0) exit(1);
    
    wait(NULL);
	//----2
	printf("\n[SA_NOCLDSTOP] (we should not be notify about SIGSTOP(19))\n");
	create_sigaction(SIGCHLD,SA_NOCLDSTOP,nocldstop_handler);
	
 	pid_t pid = fork();
 	if(pid==0) 
 	{
 		raise(SIGSTOP);//we should not be notified about it
 		exit(1);//we should be notified about it
 	}
 	else
 	{
 		sleep(1);
 	 	kill(pid, SIGCONT); 
 	 }
 	 
 	 wait(NULL);
   	//----3
    printf("\n[SA_RESETHAND] (Hello should be sent and then default SIGINT action(program end))\n");
    create_sigaction(SIGINT, SA_RESETHAND, resethand_handler);
    
    raise(SIGINT);
    sleep(1);
    raise(SIGINT);
    
    return 0;
}
