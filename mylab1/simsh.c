// simple shell example using fork() and execlp()

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
pid_t k;
char buf[100];
int status;
int len;

  while(1) {

	// print prompt
  	fprintf(stdout,"[%d]$ ",getpid());

	// read command from stdin
	fgets(buf, 100, stdin);
	len = strlen(buf);
	if(len == 1) 				// only return key pressed
	  continue;
	buf[len-1] = '\0';

  	k = fork();
  	if (k==0) {
  	// child code
    	 execlp(buf,buf,NULL); 	// if execution failed, terminate child
//    	 if(execlp(buf,buf,NULL) == -1)	// if execution failed, terminate child
//	 	exit(1);
  	}
  	else {
  	// parent code 
	 printf("parent id\n");
	 waitpid(k, &status, 0);
	printf("chile termi\n");
  	}
  }
}
