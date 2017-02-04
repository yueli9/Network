// simple shell example using fork() and execlp()

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{

int status;
int len;
char *token;
char *Cmd;

int fd;

char *msg[30];

socklen_t ctN;


if (argc < 3) {
     printf("ERROR,lacking port number or security key\n");
     exit(1);
 }

if(strlen(argv[2])<10 || strlen(argv[2])>20){
     printf("ERROR,security key length should be from 10 to 20\n");
     exit(1);
}

int sockFd;
int portNum;
int pid;
char secKey[20];
bzero(secKey,20);

struct sockaddr_in svaddr,ctaddr;

portNum = atoi(argv[1]);
strcpy(secKey,argv[2]);

memset(&svaddr, 0, sizeof(svaddr));
svaddr.sin_family = AF_INET;
//svaddr.sin_addr.s_addr = htonl(ipv4address);
svaddr.sin_addr.s_addr = INADDR_ANY;
svaddr.sin_port = htons(portNum);

sockFd = socket(AF_INET,SOCK_STREAM,0);

if(sockFd <0){
	printf("ERROR cereating socket\n");
     exit(1);
}

if(bind(sockFd,(struct sockaddr *) &svaddr, sizeof(svaddr)) <0){
	printf("ERROR binding socket\n");
	 exit(1);
}


listen(sockFd,5);

ctN = sizeof(ctaddr);
while(1) {
	int newSockFd = accept(sockFd,(struct sockaddr *) &ctaddr,&ctN);
	if(newSockFd <0){
		printf("ERROR accepting socket\n");
		exit(1);
	}

   char buf[256];
   bzero(buf,256);
   int n = read(newSockFd,buf,sizeof(buf)-1);
   buf[n] = '\0';

   	if (n < 0) {
		printf("ERROR reading socket\n");
		exit(1);
	}

  	if(n == 0)
		continue;
	
	char csecKey[20];
	
	printf("buf %s\n ",buf);
    token = strtok(buf, "$");
	strcpy(csecKey,token);  			// get secrete key from buf

	if(strcmp(csecKey,secKey) != 0){
		printf("secretkey does not match \n");
		continue;
	}

	token = strtok(NULL, "$");			// get message from buf
	printf("buf after: %s\n ",buf);

    int l=0;
	while (token != NULL) {
	printf("msg:%s",token);
	msg[l] = token;
	token = strtok(NULL, "$");
	l++;
	}
	msg[l] =NULL;
	if( strcmp(msg[0],"ls") !=0  && strcmp(msg[0],"date") !=0  && strcmp(msg[0],"host") !=0  && strcmp(msg[0],"cal") !=0 )
	{
		printf("message is %s . It can only be ls,date,host,cal.\n " );
		continue;
		//exit(1);
	}

  	int k = fork();
  	if (k==0) {							// child process
		close(sockFd);
		dup2(newSockFd,1);				//pass the output to socket
    	if(-1==execvp(msg[0],msg)){		// if execution failed, terminate child
	 		printf("excution failed");
	 		exit(1);}
	 	exit(0);
  	}	

	else if(k >0 ) {			//parent process	
	close(newSockFd);
	printf("parent prcosee \n");
	waitpid(k, &status,WNOHANG);			//wait for child process status to change
  	printf("child terminated");
	}
	else{					//fork failed exit
	printf("fork failed");	
	exit(1);
	}  
  }

close(sockFd);
return 0;
}
