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


if (argc < 7) {
     printf("ERROR,audiostreamd tcp-port udp-port payload-size packet-spacing mode logfile-s\n");
     exit(1);
 }


int tcpPort = atoi(argv[1]);
static int udpPort = atoi(argv[2])-1;
int payloadSize = atoi(argv[3]);
int packetSpacing = atoi(argv[4]);
int mode = atoi(argv[5]);
char logfileS;
strcpy(logfileS,argv[6]);


int sockFd;
struct sockaddr_in svaddr,ctaddr;
memset(&svaddr, 0, sizeof(svaddr));
memset(&svaddr, 0, sizeof(ctaddr));
ctN = sizeof(ctaddr);


svaddr.sin_family = AF_INET;
svaddr.sin_addr.s_addr = INADDR_ANY;
svaddr.sin_port = htons(tcpPort);

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


	char fb[48];
	bzero(fb,48);
	int m;
	if (access(buf,F_OK) != -1)
	{		// File exist
		udpPort++;
		sprintf(fb,"OK%d",udpPort);
		m = write(newSockFd, fb, strlen(fb));

  		int k = fork();
	  	if (k==0) {							// child process



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
	else{	// File not exist
		strcpy(fb,"KO");
		m = write(newSockFd, fb, strlen(fb));

	}





  }

close(sockFd);
return 0;
}
