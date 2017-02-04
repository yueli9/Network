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

  
    int sockFd, portNum, n;
    struct sockaddr_in svaddr,rvAddr;


    signal( SIGALRM, handle_alarm ); // Install handler first,

    char buf[1000];
    bzero(buf,1000);

    char Rbuf[256];
    bzero(Rbuf,256);

    char secKey[20];
    bzero(secKey,20);
    struct hostent *server;

    struct sockaddr_storage sStor;
    socklen_t addrSize = sizeof(sStor);
    socklen_t rvSize = sizeof(rvAddr);
    char    from_ip[1024] = "";

    if (argc < 4) {
        printf("myping hostip portnumber secretekey.\n");
        exit(0);
    }

  

    portNum = atoi(argv[2]);                                // get port number  
    strcpy(secKey,argv[3]);                                 // get host key

    sockFd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockFd <0){       
        printf("ERROR openning socket");
        exit(1);
    }

    memset(&svaddr, 0, sizeof(svaddr));

    svaddr.sin_family = AF_INET;
    //bcopy((char *)server->h_addr, (char *)&svaddr.sin_addr.s_addr, server->h_length);
    svaddr.sin_addr.s_addr = inet_addr(argv[1]);

    svaddr.sin_port = htons(portNum);

    //server = gethostbyname(argv[1]);

    //bcopy((char *)server->h_addr, (char *)&svaddr.sin_addr.s_addr, server->h_length);

    if (connect(sockFd,(struct sockaddr *) &svaddr,sizeof(svaddr)) < 0) {       
        printf("ERROR creating socket\n");
        exit(1);
    }

    printf("success create socket\n");

    //printf("string lenght %d  ,%s\n",strlen(argv[3]),argv[3]);
    
    buf[0]='$';

    int i=0;
    int sl = strlen(argv[3]);

    for(i =0;i<sl;i++){
        buf[i+1]=argv[3][i];
       // printf("%c",buf[i]);
    }
    buf[sl+1]='$';

    int c =3;
    for(i = sl+2;i<1000;i++){
        c--;
        if(c==0){
            buf[i]= '1';
            c=3;
        }
        if(c==1){
             buf[i]='A';
        }
        if(c==2){
             buf[i]='a';
        }

    }

    buf[1000] ='\0';

    struct timeval start, end, tmresult;

    ualarm(570000,900000);  //2*900000+570000 ns= 2.55s

    gettimeofday(&start, NULL);

    int srl =sendto(sockFd,buf,strlen(buf) ,0,(struct sockaddr_in *)&svaddr,sizeof(svaddr));

    //printf("origi %d  %s \n",strlen(buf),buf);

    if(srl != 1000){
        printf("Failed to send string %d, origi %d  \n",srl,strlen(buf));
    }
    else
        printf("success %d \n",srl );


    int rvt =recvfrom(sockFd,Rbuf,1024,0,(struct sockaddr *)&rvAddr, &rvSize);
    if (rvt <0)
    {
        printf("ERROR reading stock\n");
    }

   
    inet_ntop(AF_INET, &rvAddr.sin_addr, from_ip, sizeof(from_ip));


    gettimeofday(&end, NULL);
    timersub( &end, &start, &tmresult);
    
    //long double ti = (tmresult.tv_sec) * 1000+ (tmresult.tv_usec) / 1000 ;
    long double ti = (end.tv_sec -start.tv_sec) * 1000+ (end.tv_usec -start.tv_usec) / 1000 ;

    struct sockaddr_in *sin = (struct sockaddr_in *)&sStor;
    unsigned char *ip = (unsigned char *)&sin->sin_addr.s_addr;
    unsigned short int *pt = (unsigned short int *)&sin->sin_port;

    //printf("Received From: %d.%d.%d.%d Port Number %d ,time %d .\n",ip[0], ip[1], ip[2], ip[3], *pt,ti);

    printf("Received From: %s Port Number %d ,time %d ms .\n",from_ip,ntohs(rvAddr.sin_port),ti);

    close(sockFd);
    return 0;
}
