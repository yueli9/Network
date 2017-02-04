//audiolisten.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <fcntl.h>



int main(int argc, char *argv[])
{
    int sockFd, n;
    struct sockaddr_in svaddr;

    char buf[256];
    bzero(buf,256);

    char Rbuf[256];
    bzero(Rbuf,256);

    if (argc < 11) {
        printf("audiolisten server-ip server-tcp-port client-udp-port payload-size playback-del gamma buf-sz target-buf logfile-c filename .\n");
        printf("./audiolisten 128.10.3.57 57666 52833 1000  10 10 1000 1000 logC pp.au\n");
        exit(0);
    }

    char serverIP[20];
    bzero(serverIP,20);
    strcpy(serverIP,argv[1]);
    int serverTcpPort = atoi(argv[2]);                
    int clientUdpPort = atoi(argv[3]);
    int payloadSize = atoi(argv[4]);
    int playbackDel = atoi(argv[5]);
    int gamma = atoi(argv[6]);
    int bufSz = atoi(argv[7]);
    int targetBuf = atoi(argv[8]);
    char logfileC[24];
    bzero(logfileC,24);
    strcpy(logfileC,argv[9]);
    char fileName[24];
    bzero(fileName,24);
    strcpy(fileName,argv[10]);



    sockFd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockFd <0){       
        printf("ERROR openning socket");
        exit(1);
    }

    memset(&svaddr, 0, sizeof(svaddr));

    svaddr.sin_family = AF_INET;    
    svaddr.sin_port = htons(serverTcpPort);
    svaddr.sin_addr.s_addr = inet_addr(serverIP);

    if (connect(sockFd,(struct sockaddr *) &svaddr,sizeof(svaddr)) < 0) {       
        printf("ERROR creating socket");
        exit(1);
    }

    sprintf(buf, "%d %s", clientUdpPort, fileName); 

    ssize_t wlen = write(sockFd,buf,strlen(buf));
    if (wlen != strlen(buf)) 
        perror("Error: write to socket ");


    memset(&Rbuf,0,sizeof(Rbuf));

    ssize_t rlen = read(sockFd,Rbuf,sizeof(Rbuf));
    if (rlen < 0) 
         perror("ERROR reading from socket");


    printf("received:%s\n",Rbuf);

    char *pch;
    int rcvUDPport;

    pch = strtok (Rbuf," ");
    pch = strtok (NULL," ");                    // pch is portnumber
    rcvUDPport = atoi(pch);
    printf("Received portnumber %d\n",rcvUDPport );


    struct sockaddr_in udpServerAddr,udpClientAddr;
    int udpServerLen ;
    memset(&udpServerAddr, 0, sizeof(udpServerAddr));

    int udpSockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSockFd <0){       
        printf("ERROR openning udpSockFd");
        exit(1);
    }


    udpServerAddr.sin_family = AF_INET;
    udpServerAddr.sin_addr.s_addr = inet_addr(serverIP);
    udpServerAddr.sin_port = htons(rcvUDPport);

    if (connect(udpSockFd,(struct sockaddr *) &udpServerAddr,sizeof(udpServerAddr)) < 0) {       
        printf("ERROR creating udpSockFd\n");
        exit(1);
    }

    int tau = 100000;
    char sendBuffer[10];
    sprintf(sendBuffer,"%d",tau);
    int srl =sendto(udpSockFd,sendBuffer,strlen(sendBuffer) ,0,(struct sockaddr_in *)&udpServerAddr,sizeof(udpServerAddr));

    if(srl < 0){
        printf("Failed to send to server\n");
        exit(1);
    }

    char rcvBuffer[payloadSize+4];
    bzero(rcvBuffer,payloadSize+4);


    int udpRcvFd=open(fileName, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    if (udpRcvFd<0)
      {
        printf("Failed to open filename\n");
          /* code */
      }

    while(1){
        int n = recvfrom(udpSockFd, rcvBuffer, payloadSize+4, 0,(struct sockaddr_in *) &udpServerAddr, &udpServerLen);
        if (n<=0)
            break;
        printf("received from udp server:%s\n",rcvBuffer );
        char wrtBuffer[payloadSize];
        bzero(wrtBuffer,payloadSize);
        strncpy(wrtBuffer,rcvBuffer[4],n-4);
        write(udpSockFd,wrtBuffer,strlen(wrtBuffer));

    }
    close(udpRcvFd);
    close(udpSockFd);








    close(sockFd);
    return 0;
}
