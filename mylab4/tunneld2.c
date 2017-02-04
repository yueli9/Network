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
#include <stdbool.h>
#include <signal.h>

/*
void handle_alarm( int sig ) {
    
        printf("no response from real server skip\n");
        continue;
   
}

*/

// create socket for receiving from client
int createSocket(int portNum){

int tsockFd;
struct sockaddr_in svaddr;
memset(&svaddr, 0, sizeof(svaddr));

svaddr.sin_family = AF_INET;
svaddr.sin_addr.s_addr = INADDR_ANY;
svaddr.sin_port = htons(portNum);

tsockFd = socket(AF_INET,SOCK_DGRAM,0);

if(tsockFd <0){
	printf("ERROR cereating socket\n");
    exit(1);
}

if(bind(tsockFd,(struct sockaddr *) &svaddr, sizeof(svaddr)) <0){
	printf("ERROR binding socket\n");
	exit(1);
}

return tsockFd;
}

// create socket for sending to server


int connectSocket(char *intAddr,int PortNum,struct sockaddr_in *svaddr){
	//struct sockaddr_in svaddr;
	int sockFd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockFd <0){       
        printf("ERROR openning socket");
        exit(1);
    }

    memset(&(*svaddr), 0, sizeof(*svaddr));

    svaddr->sin_family = AF_INET;
    svaddr->sin_addr.s_addr = inet_addr(intAddr);
    svaddr->sin_port = htons(PortNum);

    if (connect(sockFd,(struct sockaddr *) &(*svaddr),sizeof(*svaddr)) < 0) {       
        printf("ERROR creating socket\n");
        exit(1);
    }

    printf("success create socket to real server\n");
    return sockFd;

}


int main(int argc, char *argv[])
{


struct sockaddr_in rvAddr[5];
socklen_t rvSize = sizeof(rvAddr[0]);

int status;
int len;
char *token;
char severIP[50];
bzero(severIP,50);

char Sseckey[20] = "ddmt7OAOOxMLa";
char buf[1000];
bzero(buf,1000);

char tbuf[1024];
bzero(tbuf,1024);

char sbuf[1000] ;
bzero(sbuf,1000);

struct sockaddr_storage sStor,rvStore;

struct sockaddr_storage clientStore[5],serverStore[5];

socklen_t addrSize = sizeof(sStor);

 	char serverIP[20];  // actual server IP received
    bzero(serverIP,20);

    char serverPort[10];      //second vpn port number from vpn server
    bzero(serverPort,10);


if (argc < 2) {
     printf("ERROR,lacking port number \n");
     exit(1);
 }

int i =0;


int sockFd = createSocket(atoi(argv[1]));// create vpn socket;
int svPt[5] ={57343,56893,59283,59334,53945};
int svSocket[5];
bool svAvail[5];
int rsvPort[5];
char rsvIP[5][48];
int k = 0;
int realsvSocket[5];
struct sockaddr_in realServAddr[5];   // store address for real server
struct sockaddr_in realServStore[5];   // store address for real server

//struct sockaddr_in vaddr;
for (k =0;k<5;k++){
	svSocket[k] = createSocket(svPt[k]);	// create socket for real client
	svAvail[k] = true;
    rsvPort[k] = 0;
    bzero(rsvIP[k],48);
    realsvSocket[k] = 0;
}

int tn = 4;

char    sev_ip[1024] = "";

//inet_ntop(AF_INET, &svaddr.sin_addr, sev_ip, sizeof(sev_ip));
//printf("server ip: %s Port Number %d \n",sev_ip,ntohs(svaddr.sin_port));

//ctN = sizeof(ctaddr);

//unsigned char *ip = svaddr.sin_addr.s_addr;
//printf("Received From: %d.%d.%d.%d .\n",ip[0], ip[1], ip[2], ip[3]);

while(1) {
    //printf("Waiting for new msg\n");
	int nb = recvfrom(sockFd,buf,1000,MSG_DONTWAIT,(struct sockaddr *)&rvStore, &addrSize);
   	
    if(nb >0){

	printf("buf length %d,%s\n ",nb,buf);
    token = strtok(buf, "$");

	strcpy(severIP, token);  		

	token = strtok(NULL, "$");				// get IP and port
    printf("IP %s, port %s \n",severIP,token);




	for (k =0;k<5;k++){
	if (svAvail[k] == true)
		{
			 int ln = sprintf(sbuf,"$%s$%d",severIP,svPt[k]);
             sbuf[ln] ='\0';
             printf("send to vpn client : %s  length:%d\n",sbuf,strlen(sbuf));
			 int sbs = sendto(sockFd,sbuf,strlen(sbuf),0,(struct sockaddr *)&rvStore,addrSize);
             if(sbs <0){
                printf("Failed to send back to vpn client %d \n",sbs);
                exit(0);
             }
             printf("send back size %d \n",sbs);
		     bzero(sbuf,100);
             rsvPort[k] = atoi(token);
             strcpy(rsvIP[k] , severIP);
             realsvSocket[k]  = connectSocket(severIP,atoi(token),&realServAddr[k]); // connect to real server
            svAvail[k] = false;


            struct timeval tv;

            tv.tv_sec = 3;  /* 30 Secs Timeout */
            tv.tv_usec = 0;  // Not init'ing this can cause strange errors

            setsockopt(realsvSocket[k], SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));  // set 3s timeout

			printf("The %d Port Number is assigned\n",k);
            break;


		}

	}

    }

    else{

    for (i =0;i<5;i++){
        if(  svAvail[i] == false){

        bzero(tbuf,1024);

        int tnb = recvfrom(svSocket[i],tbuf,1024,0,(struct sockaddr *)&clientStore[i], &addrSize);
        if(tnb <0){
        printf("ERROR receiving from UDP socket\n");
        exit(1);
            }
        if(tnb == 0)
            continue;

        printf("buf length %d,%s\n ",tnb,tbuf);

        int srl =sendto(realsvSocket[i],tbuf,strlen(tbuf) ,0,(struct sockaddr_in *)&realServAddr[i],sizeof(realServAddr[i]));  // send to real server
        if(srl <0){
        printf("ERROR sending to real UDP server\n");
        exit(1);
            }
        
        char rsbuf[1024];
        bzero(rsbuf,1024);


        socklen_t tmpSize = sizeof(realServAddr[i]);

        printf("waiting for response from real server\n");
        //int rsbl = recvfrom(realsvSocket[i],rsbuf,1024,0,(struct sockaddr_in *)&realServAddr[i], &tmpSize);

        //signal( SIGALRM, handle_alarm ); // Install handler first,


        int rsbl = recvfrom(realsvSocket[i],rsbuf,1024,0,(struct sockaddr *)&(realServAddr[i]), &tmpSize);



        if(rsbl <0){
        printf("TIMEOUT receiving from real UDP server\n");  // timeout after 3 s
        //exit(1);
        continue;
            }
        if(rsbl == 0)
            continue;

        printf("Receive from real server,buf length %d,%s\n ",rsbl,rsbuf);                   // send to client

        int rcbl =sendto(svSocket[i],rsbuf,strlen(rsbuf) ,0,(struct sockaddr_in *)&clientStore[i],sizeof(clientStore[i]));
        if(rcbl <0){
        printf("ERROR sending to real UDP client\n");
        exit(1);
            }

        //printf("VPN circle finish \n");

        }
        //printf("Outer VPN circle finish \n");

        }
	//printf("success sent back!\n");

    }

  }

close(sockFd);
return 0;
}
