#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/time.h>
#include <signal.h>
#include <setjmp.h>
#include <stdbool.h>

#define CMD       "ls"


static st =0;
void handle_alarm( int sig ) {
    st++;
    if(st ==3){
        printf("no response from ping server\n");
        exit(1);
    }
   
}

int main(int argc, char *argv[])
{

  
    int sockFd, vpnPortNum, n;
    struct sockaddr_in svaddr,rvAddr;

    char *token;

    signal( SIGALRM, handle_alarm ); // Install handler first,

    char buf[1000];
    bzero(buf,1000);

    char Rbuf[1024];
    bzero(Rbuf,1024);

    char serverIP[20];  // actual server IP received
    bzero(serverIP,20);

    char vpnPort[10];      //second vpn port number from vpn server
    bzero(vpnPort,10);

    struct hostent *server;

    struct sockaddr_storage sStor;
    socklen_t addrSize = sizeof(sStor);
    socklen_t rvSize = sizeof(rvAddr);
    char    from_ip[1024] = "";

    if (argc < 5) {
        printf("mytunnel vpn-IP vpn-port server-IP server-port-number.\n");
        exit(0);
    }

  

    vpnPortNum = atoi(argv[2]);                                // get vpn port number  

    sockFd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockFd <0){       
        printf("ERROR openning socket");
        exit(1);
    }

    memset(&svaddr, 0, sizeof(svaddr));

    svaddr.sin_family = AF_INET;
    //bcopy((char *)server->h_addr, (char *)&svaddr.sin_addr.s_addr, server->h_length);
    svaddr.sin_addr.s_addr = inet_addr(argv[1]);

    svaddr.sin_port = htons(vpnPortNum);

    //server = gethostbyname(argv[1]);

    //bcopy((char *)server->h_addr, (char *)&svaddr.sin_addr.s_addr, server->h_length);

    if (connect(sockFd,(struct sockaddr *) &svaddr,sizeof(svaddr)) < 0) {       
        printf("ERROR creating socket\n");
        exit(1);
    }

    printf("success create socket to VPN server\n");

    //printf("string lenght %d  ,%s\n",strlen(argv[3]),argv[3]);
    
    buf[0]='$';

    int i=0,j=0;
    int sl = strlen(argv[3]);

    for(i =0;i<sl;i++){
        buf[i+1]=argv[3][i];
       // printf("%c",buf[i]);
    }
    buf[sl+1]='$';

    int psl = strlen(argv[4]);  // get port string length

    for(j = 0;j<psl;j++){
    
    buf[sl+1+1+j]=argv[4][j];  // attach port number

    }

    buf[sl+1+1+psl] ='\0';

    struct timeval start, end, tmresult;


    int srl =sendto(sockFd,buf,strlen(buf) ,0,(struct sockaddr_in *)&svaddr,sizeof(svaddr));

    //printf("origi %d  %s \n",strlen(buf),buf);

    if(srl == 0){
        printf("Failed to send string %d, origi %d  \n",srl,strlen(buf));
    }





    int rvt =recvfrom(sockFd,Rbuf,1024,0,(struct sockaddr *)&rvAddr, &rvSize);
    if (rvt <0)
    {
        printf("ERROR reading stock\n");
    }

    if(rvt >0){
    printf("reveive msg  %d\n",rvt);



    printf("received msg: %s\n ",Rbuf);
    token = strtok(Rbuf, "$");
    strcpy(serverIP, token);          
    // get seckey from buf

    token = strtok(NULL, "$");              // get message from buf
    //printf("msg is: %s\n ",buf);
    strcpy(vpnPort, token);          



    printf("The second vpn port number: %s \n",vpnPort);
    }
    
/*
   
    inet_ntop(AF_INET, &rvAddr.sin_addr, from_ip, sizeof(from_ip));


    struct sockaddr_in *sin = (struct sockaddr_in *)&sStor;
    unsigned char *ip = (unsigned char *)&sin->sin_addr.s_addr;
    unsigned short int *pt = (unsigned short int *)&sin->sin_port;

    //printf("Received From: %d.%d.%d.%d Port Number %d ,time %d .\n",ip[0], ip[1], ip[2], ip[3], *pt,ti);

    printf("Received From: %s Port Number %d .\n",from_ip,ntohs(rvAddr.sin_port));
*/
    //close(sockFd);
    return 0;
}
