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

if (argc < 2) {
     printf("ERROR,lacking port number \n");
     exit(1);
 }


int sockFd;
int portNum;
int pid;
char secKey[20];
bzero(secKey,20);

struct sockaddr_in svaddr,ctaddr;

portNum = atoi(argv[1]);

memset(&svaddr, 0, sizeof(svaddr));
svaddr.sin_family = AF_INET;
//svaddr.sin_addr.s_addr = htonl(ipv4address);
svaddr.sin_addr.s_addr = INADDR_ANY;
svaddr.sin_port = htons(portNum);

sockFd = socket(AF_INET,SOCK_DGRAM,0);

if(sockFd <0){
	printf("ERROR cereating socket\n");
     exit(1);
}


if(bind(sockFd,(struct sockaddr *) &svaddr, sizeof(svaddr)) <0){
	printf("ERROR binding socket\n");
	 exit(1);
}
int tn = 4;

char    sev_ip[1024] = "";

inet_ntop(AF_INET, &svaddr.sin_addr, sev_ip, sizeof(sev_ip));
printf("server ip: %s Port Number %d \n",sev_ip,ntohs(svaddr.sin_port));

//ctN = sizeof(ctaddr);

//unsigned char *ip = svaddr.sin_addr.s_addr;
//printf("Received From: %d.%d.%d.%d .\n",ip[0], ip[1], ip[2], ip[3]);

while(1) {
	int nb = recvfrom(sockFd,buf,1000,0,(struct sockaddr *)&sStor, &addrSize);
   	if(nb <0){
   		printf("ERROR receiving from UDP socket\n");
     	exit(1);
   	}
  	if(nb == 0)
		continue;
	if (nb != 1000)
	{
		printf("MSG: %s",buf);
		printf("Error,message length is not 1000 ! %d \n", strlen(buf));
		continue;
	}
	printf("buf length %d,%s\n ",nb,buf);
    token = strtok(buf, "$");
	strcpy(secKey, token);  		
	// get seckey from buf
	int i;


	if(strcmp(secKey,Sseckey) !=0 ){
		printf("Error,secete key doe not match old %s new %s \n",secKey,Sseckey);
		continue;
	}

	token = strtok(NULL, "$");				// get message from buf
	//printf("msg is: %s\n ",buf);
	
	tn --;
	if(tn == 0 ){
		tn =4;
		printf("skip this msg\n");
		continue;
	}

	int sbs = sendto(sockFd,sbuf,strlen(sbuf),0,(struct sockaddr *)&sStor,addrSize);
	if(sbs <0){
		printf("Failed to send back\n");	}

	printf("success sent back! %d \n",sbs);
  }

close(sockFd);
return 0;
}
