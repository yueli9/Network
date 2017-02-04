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



int main(int argc, char *argv[])
{

  
    int sockFd, portNum, n;
    struct sockaddr_in svaddr,rvAddr;

    char Rbuf[256];
    bzero(Rbuf,256);

    char secKey[20];
    bzero(secKey,20);
    struct hostent *server;

    struct sockaddr_storage sStor;
    socklen_t addrSize = sizeof(sStor);
    socklen_t rvSize = sizeof(rvAddr);
    char    from_ip[1024] = "";

    if (argc < 6) {
        printf("traffic_snd IP-address port-number payload-size packet-count packet-spacing\n");
        exit(0);
    }

  

    portNum = atoi(argv[2]);                                // get port number  
    strcpy(secKey,argv[3]);                                 // get host key

    int payloadSize = atoi(argv[3]);                    // get payload-size 
    int packetCount = atoi(argv[4]);                    // get packet-count 
    float packetSpace = atof(argv[5]);                    // get packet-spacing 
    //double packetSpace = strtod(argv[5],NULL);
    int slpusc = (int) packetSpace*1000;                // convert to usec

    sockFd = socket(AF_INET, SOCK_DGRAM, 0);            // create socket

    if (sockFd <0){       
        printf("ERROR openning socket");
        exit(1);
    }

    memset(&svaddr, 0, sizeof(svaddr));                         // config stocket

    svaddr.sin_family = AF_INET;

    svaddr.sin_addr.s_addr = inet_addr(argv[1]);

    svaddr.sin_port = htons(portNum);

    if (connect(sockFd,(struct sockaddr *) &svaddr,sizeof(svaddr)) < 0) {     // connect socket  
        printf("ERROR creating socket\n");
        exit(1);
    }

    printf("success create socket\n");

    
    char buf[payloadSize+1];
    bzero(buf,payloadSize+1);
    int i =0;
    for (i = 0;i<payloadSize;i++){                               // build up msg to send.
        buf[i]='L';
    }
    buf[payloadSize] ='\0';


    char stopbuf[3] ="SSS";                                     // stop msg is "SSS"

    struct timeval start, end, tmresult;

    gettimeofday(&start, NULL);                                 // record start time

    int j =0;
    int pc =0;
    int bc = 0;
    for (j =0;j<packetCount;j++){                                // send msg fot server
    int srl =sendto(sockFd,buf,strlen(buf) ,0,(struct sockaddr_in *)&svaddr,sizeof(svaddr)); 

    //printf("origi %d  %s \n",strlen(buf),buf);

    if(srl != payloadSize){
        printf("Failed to send string %d, origi %d  \n",srl,strlen(buf));   // check payload size

    }
    else{
        pc++;
        bc = bc+srl+46;
        printf("total packet sent %d \n",pc);
    }
    usleep(slpusc);                                         // sleep for usc

    }
    // signal receiver to  stop 
    int k = 0;
    for (k =0;k<3;k++){
    int ssrl =sendto(sockFd,stopbuf,strlen(stopbuf) ,0,(struct sockaddr_in *)&svaddr,sizeof(svaddr));
    if(ssrl != 3){
        printf("Failed to send string %d, origi %d  \n",ssrl,strlen(stopbuf));
    }
    }

    gettimeofday(&end, NULL);                                       // record stop time
    timersub( &end, &start, &tmresult);
    
    long double ti = (end.tv_sec -start.tv_sec) * 1000+ (end.tv_usec -start.tv_usec) / 1000 ;
    long double ts = ti/1000.0;                                     //conver to sec

    printf("\n Completion time %Lf,reliable bps %Lf, pps %Lf.\n",ts,bc/ts,pc/ts);


    close(sockFd);
    return 0;
}
