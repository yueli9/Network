// simple audiostreamd.c


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/file.h>   /* for O_NONBLOCK and FASYNC */
#include <signal.h>     /* for signal() and SIGALRM */
#include <errno.h>      /* for errno */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <netdb.h> 

void SIGIOHandler(int signalType);      //  SIGIO handler
static int udpSockFd ;
static int tau;							// tau updated by sigpoll


int main(int argc, char *argv[])
{

int status;

socklen_t ctN;


if (argc < 7) {
    printf("ERROR,audiostreamd tcp-port udp-port payload-size packet-spacing mode logfile-s\n");
    printf("./audiostreamd 57666 58933 1000 100 1 logs\n");
	exit(1);
 }


int tcpPort = atoi(argv[1]);
int udpPort = atoi(argv[2])-1;
int payloadSize = atoi(argv[3]);
int packetSpacing = atoi(argv[4]);
int mode = atoi(argv[5]);
char logfileS[20];
bzero(logfileS,20);
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

	printf("Rceived:%s\n",buf );
	char *pch;
	int rcvUDPport;

	pch = strtok (buf," ");
	rcvUDPport = atoi(pch);
    pch = strtok (NULL," ");  					// pch is filename now


	char fb[48];
	bzero(fb,48);
	int m;
	if (access(pch,F_OK) != -1)
	{											// File exist
		udpPort++;
		sprintf(fb,"OK %d",udpPort);
		m = write(newSockFd, fb, strlen(fb));

  		int k = fork();
	  	if (k==0) {								// child process

	  	tau  = 0;

		struct sockaddr_in udpServerAddr,udpClientAddr;
		memset(&udpServerAddr, 0, sizeof(udpServerAddr));
		memset(&udpClientAddr, 0, sizeof(udpClientAddr));
		socklen_t udpClientln = sizeof(udpServerAddr);
		char rcvBuffer[1024];
		bzero(rcvBuffer,1024);


		udpServerAddr.sin_family = AF_INET;
		udpServerAddr.sin_addr.s_addr = INADDR_ANY;
		udpServerAddr.sin_port = htons(udpPort);

		udpSockFd = socket(AF_INET,SOCK_DGRAM,0);
		if(udpSockFd <0){
			printf("ERROR cereating udpSockFd\n");
     		exit(1);
		}
		if(bind(udpSockFd,(struct sockaddr *) &udpServerAddr, sizeof(udpServerAddr)) <0){
		printf("ERROR binding udpSockFd\n");
	 	exit(1);
		}


   		if (recvfrom(udpSockFd, rcvBuffer, 1024, 0, (struct sockaddr_in *) &udpClientAddr, &udpClientln) < 0){
        	printf("Failed receive from client\n");
        	exit(1);
        	}

        printf("received:%s\n",rcvBuffer );
    	tau = atoi(rcvBuffer);
		//tau = packetSpacing;						// initial packet spacing

		struct sigaction handler;        				//Signal handling action definition 

    	handler.sa_handler = SIGIOHandler;
    	if (sigfillset(&handler.sa_mask) < 0) 
        	{printf("sigfillset() failed");
            exit(1);}

    	handler.sa_flags = 0;           				// no flags

    	if (sigaction(SIGIO, &handler, 0) < 0)
        	{printf("sigaction() failed for SIGIO");
        	exit(1);}

    	if (fcntl(udpSockFd, F_SETOWN, getpid()) < 0)
        	{printf("Unable to set process owner to us");
        	exit(1);}

    	if (fcntl(udpSockFd, F_SETFL, O_NONBLOCK | FASYNC) < 0)
        	{printf("Unable to put client sock into non-blocking/async mode");
            	exit(1);}


        char readBuffer[payloadSize];
        bzero(readBuffer,payloadSize);

        printf("%s\n",pch );
    	int fd = open(pch, O_RDONLY);
    	int i = 0;
    	while(1){


    		int rn = read(fd, readBuffer, payloadSize);
    		if (rn<=0)
    			break;

    		char sendbackBuffer[payloadSize+4];
    		bzero(sendbackBuffer,payloadSize+4);
    		sprintf(sendbackBuffer,"%04d%s",i,readBuffer);

    		//printf("%s\n",sendbackBuffer );
			if(sendto(udpSockFd,sendbackBuffer,strlen(sendbackBuffer),0,(struct sockaddr *)&udpClientAddr,udpClientln) <0){
			printf("Failed to send back\n");	}

			i++;
			usleep(tau);



    	}
    	close(udpSockFd);
    	close(newSockFd);





	  	}	

		else if(k >0 ) {						//parent process	
		//close(newSockFd);
		printf("parent prcosee \n");
		waitpid(k, &status,WNOHANG);			//wait for child process status to change
	  	printf("child terminated");
		}
		else{									//fork failed exit
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



void SIGIOHandler(int signalType)
{
    struct sockaddr_in sndAddr;     		/* Address of sender */
    int sndAdLn = sizeof(sndAddr);          
    int recSz;                  
    char rcvBuf[1024];       

    if ((recSz = recvfrom(udpSockFd, rcvBuf, 1024, 0, (struct sockaddr *) &sndAddr, &sndAdLn)) < 0)
        {
        	printf("Failed receive from client\n");
        	exit(1);
        }

    tau = atoi(rcvBuf);


}
