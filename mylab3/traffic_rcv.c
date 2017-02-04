// simple UDP ping server

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
					
char Sseckey[20] = "ddmt7OAOOxMLa";
char buf[1000];
bzero(buf,1000);
char sbuf[5] = "terve";

struct sockaddr_storage sStor;
socklen_t addrSize;

if (argc < 3) {
     printf("ERROR,lacking port number payload size\n");
     exit(1);
 }


int sockFd;
int portNum;
int pid;
char secKey[20];
bzero(secKey,20);

struct sockaddr_in svaddr,ctaddr;

portNum = atoi(argv[1]);
int payloadSize = atoi(argv[2]);


memset(&svaddr, 0, sizeof(svaddr));
svaddr.sin_family = AF_INET;
//svaddr.sin_addr.s_addr = htonl(ipv4address);
svaddr.sin_addr.s_addr = INADDR_ANY;											// config address
svaddr.sin_port = htons(portNum);

sockFd = socket(AF_INET,SOCK_DGRAM,0);

if(sockFd <0){
	printf("ERROR cereating socket\n");
     exit(1);
}


if(bind(sockFd,(struct sockaddr *) &svaddr, sizeof(svaddr)) <0){				// set up socket
	printf("ERROR binding socket\n");
	 exit(1);
}
int tn = 4;

char    sev_ip[1024] = "";

inet_ntop(AF_INET, &svaddr.sin_addr, sev_ip, sizeof(sev_ip));
printf("server ip: %s Port Number %d \n",sev_ip,ntohs(svaddr.sin_port));		// get ip and port number


struct timeval start, end, tmresult;


int pc = 0; // count the packet number
int bc = 0 ; // count the byte number
int ss =0;

while(1) {
	int nb = recvfrom(sockFd,buf,100000,0,(struct sockaddr *)&sStor, &addrSize); 	// receive msg from sender.
   	if(nb <0){
   		printf("ERROR receiving from UDP socket\n");								// check error
     	exit(1);
   	}
  	if(nb == 0)
		continue;
	if (nb >3 )
	{
	if(pc ==0){
		gettimeofday(&start, NULL);				// record when got the first packet
		}
		pc++;
		bc = bc+46+nb;							// IP+UDP+ethernet header 46
	
	printf("buf length %d.\n ",nb);


	}
	if(nb == 3){

		ss++;
		if(ss ==3){								// detect stop signal
    	gettimeofday(&end, NULL);

   		long double ti = (end.tv_sec -start.tv_sec) * 1000+ (end.tv_usec -start.tv_usec) / 1000 ;
   		long double ts = ti/1000.0;				//conver to sec

   		printf("\n Completion time %Lf,reliable bps %Lf, pps %Lf.\n",ts,bc/ts,pc/ts);


   		bc =0; 									// reset to zeros for bc pc
   		pc =0;
		ss =0;
		//close(sockFd);
		//exit(1);
		}
	}

	}

 
close(sockFd);
return 0;
}
