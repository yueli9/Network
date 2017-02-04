// simple UDP based wetalk app

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


static void sig_alrm(int);
int recvSockFd;                     // recvSockFd is for receiving from peer,make global for sigio
void SIGIOHandler(int signalType);      //  SIGIO handler

int main(int argc, char *argv[])
{

struct sigaction handler;        /* Signal handling action definition */

int status;
int len;
char *token;

char buf[1000];
bzero(buf,1000);

struct sockaddr_storage sStor;
socklen_t addrSize = sizeof(sStor);

if (argc < 2) {
     printf("ERROR,lacking port number \n");
     exit(1);
 }

int portNum;
int pid;

struct sockaddr_in svaddr,ctaddr,rvAddr;

socklen_t rvSize = sizeof(rvAddr);

portNum = atoi(argv[1]);

memset(&svaddr, 0, sizeof(svaddr));
svaddr.sin_family = AF_INET;
//svaddr.sin_addr.s_addr = htonl(ipv4address);
svaddr.sin_addr.s_addr = INADDR_ANY;
svaddr.sin_port = htons(portNum);

recvSockFd = socket(AF_INET,SOCK_DGRAM,0);

if(recvSockFd <0){
    printf("ERROR cereating socket\n");
     exit(1);
}


if(bind(recvSockFd,(struct sockaddr *) &svaddr, sizeof(svaddr)) <0){
    printf("ERROR binding socket\n");
     exit(1);
}

int serverMode = 1;  // serverMode receive msg first
int beforeNormal = 1; // before establish normal converservation


int aWait = 1;

while(aWait == 1)
{

if (beforeNormal==1)
{
    /* code */
    printf("?\n"); // wait for input

}
else{                   
        printf(">\n"); // wait for input

}

char initMsg[50];
bzero(initMsg,50);

char sndMsg[10] = "wannatalk";


char hostName[30];
bzero(hostName,30);
int hostPortNum =0;
char Rbuf[3];

signal(SIGALRM, sig_alrm);  // register the alarm handler
siginterrupt(SIGALRM, 1);


    handler.sa_handler = SIGIOHandler;
    if (sigfillset(&handler.sa_mask) < 0) 
        {printf("sigfillset() failed");
            exit(1);}

    handler.sa_flags = 0;           // no flags

    if (sigaction(SIGIO, &handler, 0) < 0)
        {printf("sigaction() failed for SIGIO");
        exit(1);}

    if (fcntl(recvSockFd, F_SETOWN, getpid()) < 0)
        {printf("Unable to set process owner to us");
        exit(1);}

    if (fcntl(recvSockFd, F_SETFL, O_NONBLOCK | FASYNC) < 0)
        {printf("Unable to put client sock into non-blocking/async mode");
            exit(1);}



scanf("%s",initMsg);  // take input msg


if (strcmp(initMsg,"q")==0)
{
    printf("Input %s, exit from wetalk\n",initMsg );
    exit(1);
}

else{


int sndSockFd = socket(AF_INET,SOCK_DGRAM,0);  // sndSockFd is for sending to peer


//strcpy(sndMsg,initMsg);                         // preserve msg to send
token = strtok(initMsg, "$");
strcpy(hostName,token);
token = strtok(NULL, "$");                      // get message from initMsg
hostPortNum=atoi(token);
// build socket


struct hostent *server;

server = gethostbyname(hostName);
if (server == NULL){                           // get server name
        printf("ERROR no such hostname \n");
        exit(1);
}


memset(&ctaddr, 0, sizeof(ctaddr));
ctaddr.sin_family = AF_INET;
//ctaddr.sin_addr.s_addr = htonl(ipv4address);
bcopy((char *)server->h_addr, (char *)&ctaddr.sin_addr.s_addr, server->h_length);

//ctaddr.sin_addr.s_addr = INADDR_ANY;
ctaddr.sin_port = htons(hostPortNum);

//sockFd = socket(AF_INET,SOCK_DGRAM,0);

if (connect(sndSockFd,(struct sockaddr *) &ctaddr,sizeof(ctaddr)) < 0) {       
        printf("ERROR creating socket\n");
        exit(1);
}

printf("success create socket to peer\n");



int srl =sendto(sndSockFd,sndMsg,strlen(sndMsg) ,0,(struct sockaddr_in *)&ctaddr,sizeof(ctaddr));

if(srl<0){
    printf("Failed to send to peer wetalk\n");
    exit(1);

}

alarm(7);  // alarm after 7 seconds


bzero(Rbuf,sizeof(Rbuf));

int rvt = -1;
rvt =recvfrom(sndSockFd,Rbuf,2,0,(struct sockaddr *)&rvAddr, &rvSize);
if (rvt <0)
{
        printf("ERROR reading stock or TimeOut\n");
}
else{
    printf("Reset Alarm\n");
    alarm(0);  // reset alarm
}

printf("Received %s\n",Rbuf );
if (Rbuf[0] == 'O' && Rbuf[1] == 'K')
{


printf(">\n"); //include \n??

char tmpsb[51];

scanf("%s",tmpsb);

}
else if (Rbuf[1] == 'O' && Rbuf[0] == 'K')
{
printf("| doesn't want to chat\n");
continue;
}

}

}


close(recvSockFd);
return 0;
}




static void sig_alrm(int signo)
{

    printf("TimeOut\n");
     return;                     /* just interrupt the recvfrom() */
 }




 void SIGIOHandler(int signalType)
{
    struct sockaddr_in sndAddr;     /* Address of sender */
    int sndAdLn = sizeof(sndAddr);          
    int recSz;                  
    char rcvBuf[1024];       


    do
    {

        if ((recSz = recvfrom(recvSockFd, rcvBuf, 1024, 0, (struct sockaddr *) &sndAddr, &sndAdLn)) < 0)
        {

            printf("failed recvfrom %d\n",recSz);
            /* Only acceptable error: recvfrom() would have blocked */
            if (errno != EWOULDBLOCK)  
            {
                printf("recvfrom failed\n");
                exit(1);
            }
        }
        else
        {

            if (strcmp(rcvBuf,"wannatalk")==0)
            {
                char from_ip[50];
                inet_ntop(AF_INET, &sndAddr.sin_addr, from_ip, sizeof(from_ip));
                int ptn = ntohs(sndAddr.sin_port);

                printf("|chat request from %s : %d\n?\n",from_ip,ptn );
                char resb[51];
                scanf("%s",resb);
                char feedb[2];


                if(strcmp("c",resb)==0)
                {
                    feedb[0] ='O';
                    feedb[1] ='K';
                

                }

                if(strcmp("n",resb)==0)
                {
                    feedb[0] ='K';
                    feedb[1] ='O';

                }
                //feedb[2]='\0';

                int febl =sendto(recvSockFd,feedb,strlen(feedb) ,0,(struct sockaddr_in *)&sndAddr,sizeof(sndAddr));

                if(febl <0)
                {
                    printf("Failed sendback \n");
                    exit(1);
                }

                if(strcmp("OK",feedb) == 0){



                    int newConv = 1;
                    while(newConv ==1){
                    printf(">\n");
                    char tresb[51];
                    bzero(tresb,51);
                    scanf("%s",tresb);
                    //int nwcl =sendto(recvSockFd,tresb,strlen(tresb) ,0,(struct sockaddr_in *)&sndAddr,sizeof(sndAddr));

                    }

                if(strcmp("KO",feedb) == 0){
                    return;

                    //int nwcl =sendto(recvSockFd,tresb,strlen(tresb) ,0,(struct sockaddr_in *)&sndAddr,sizeof(sndAddr));

                    }



                }




            }


        }
    }  while (recSz >= 0);

                //printf("Outerlayer\n");

    /* Nothing left to receive */
}
