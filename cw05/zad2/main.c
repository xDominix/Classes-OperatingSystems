#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void error(char *msg) { printf("%s", msg); exit(0); }

void mailByDate()
{
	char command[50];
	sprintf(command, "a\na");
	
	FILE * mail = popen("mail -N sort date", "w");
	fputs(command, mail);
	
	pclose(mail);
}

void mailByFrom()
{
	char command[50];
	sprintf(command, "a\na");
	
	FILE * mail = popen("mail -N sort from", "w");
	fputs(command, mail);
	
	pclose(mail);
}


void mailSend(char* to,char* subject, char* message) //mail -s "This is the subject" somebody@example.com <<< 'This is the message'
{
	char command[100];
	sprintf(command, "mail -s \"%s\" %s", subject, to);
	
	FILE * mail = popen(command, "w");
	fputs(message, mail);
	
	pclose(mail);
}

int main(int argc, char ** argv){
	if(argc == 2)
	{
		if(strcmp(argv[1], "data") == 0) mailByDate();
		else if(strcmp(argv[1], "nadawca") == 0) mailByFrom();
		else error("Invalid arguments\n");
	}
	else if(argc == 4) mailSend(argv[1], argv[2], argv[3]);
	else error("Wrong no arguments\n");
	
	return 0;
}
