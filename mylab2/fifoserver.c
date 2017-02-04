// simple shell example using fork() and execlp()

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>



int main(void)
{
pid_t k;

pid_t cpid;
char buf[100];
int status;
int len;
char *token;
char *Cmd;

char cfifo[sizeof "cfifo11111"];
int cfd;
int fd;

char *sfifo = "cmdfifo";			// server create cmdfifo
mkfifo(sfifo,0666);	
  
while(1) {
	printf("open sfifo");
	fd = open(sfifo,O_RDONLY);		//open the cmdfifo and read the message
	printf("file id %d \n",fd);
	read(fd, buf, 100); 			// store message in buf
	close(fd);				// close sfifo
	printf("buf %s\n ",buf);
     	token = strtok(buf, "$");
	cpid = atoi(token);  			// get client pid from buf
	token = strtok(NULL, "$");		// get message from buf
	printf("buf after: %s\n ",buf);
	
	len = strlen(token);
	if(len == 1) 				// only return key pressed
	  continue;

	printf("cpid %d \n",cpid);
	sprintf(cfifo,"cfifo%d",cpid);		//get the client fifo name			
	cfd = open(cfifo,O_WRONLY);		//open client fifo file

  	k = fork();
  	if (k==0) {				// child process
	dup2(cfd,1);				//pass the output to client fifo
    	if(-1==execlp(token,token,NULL));	// if execution failed, terminate child
	 	exit(1);
	close(cfd);				// close client fifo
  	}	
	else if(k >0 ) {			//parent process	
	//else{
	printf("parent prcosee \n");
	//waitpid(k, &status,0);			//wait for child process status to change
	waitpid(k, &status,WNOHANG);			//wait for child process status to change
  	printf("child terminated");
	}
	else{					//fork failed exit
	printf("fork failed");	
	exit(1);
	}  
  close(cfd);
  close(fd);
  }
unlink(cfd);
return 0;
}
