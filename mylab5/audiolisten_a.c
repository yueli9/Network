//audiolisten.c
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
#include <pthread.h>
#include <semaphore.h>


static int bufSz;
//void *b[N];
int in = 0;
int out = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static sem_t countsem;
static sem_t spacesem;

char semBuffer[500000];

void SIGIOHandler(int signalType);      //  SIGIO handler
void handle_alarm(int sig);             //  SIGIO handler

void enqueue(char value);
void dequeue();

static int udpSockFd;
static int payloadSize;

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
        printf("./audiolisten 128.10.3.57 57666 52833 1000  10 10 4000 1000 logC tmp/pp.au\n");
        exit(0);
    }

    char serverIP[20];
    bzero(serverIP,20);
    strcpy(serverIP,argv[1]);
    int serverTcpPort = atoi(argv[2]);                
    int clientUdpPort = atoi(argv[3]);
    payloadSize = atoi(argv[4]);
    int playbackDel = atoi(argv[5]);
    int gamma = atoi(argv[6]);
    bufSz = atoi(argv[7]);
    int targetBuf = atoi(argv[8]);
    char logfileC[24];
    bzero(logfileC,24);
    strcpy(logfileC,argv[9]);
    char fileName[24];
    bzero(fileName,24);
    strcpy(fileName,argv[10]);


    //bzero(semBuffer,500000);

    sem_init(&countsem, 0, 0);
    sem_init(&spacesem, 0, bufSz);

    printf("!!!countsem %d spacesem %d\n",countsem ,spacesem );


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

    udpSockFd = socket(AF_INET, SOCK_DGRAM, 0);
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

    int tau = 1000;
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
    printf("Wait fro msg\n");


    signal( SIGALRM, handle_alarm );                // Install handler for ualarm,


    struct sigaction handler;                       //Signal handling action definition 

    handler.sa_handler = SIGIOHandler;
    if (sigfillset(&handler.sa_mask) < 0) 
        {printf("sigfillset() failed");
        exit(1);}

    handler.sa_flags = 0;                           // no flags

    if (sigaction(SIGIO, &handler, 0) < 0)
        {printf("sigaction() failed for SIGIO");
        exit(1);}

    if (fcntl(udpSockFd, F_SETOWN, getpid()) < 0)
        {printf("Unable to set process owner to us");
        exit(1);}

    if (fcntl(udpSockFd, F_SETFL, O_NONBLOCK | FASYNC) < 0)
        {printf("Unable to put client sock into non-blocking/async mode");
            exit(1);}

    
    ualarm(100,10000);  //2*900000+570000 ns= 2.55s


    while(1){
       
        /* int n = recvfrom(udpSockFd, rcvBuffer, payloadSize+4, 0,(struct sockaddr_in *) &udpServerAddr, &udpServerLen);
        if (n<=0)
            { printf("ERROR receive from server\n");
            break;
        }
        //printf("received from udp server:%s\n",rcvBuffer );
        char wrtBuffer[payloadSize];
        bzero(wrtBuffer,payloadSize);
        printf("n is :%d\n",n );
        strncpy(wrtBuffer,rcvBuffer+4,n-4);
        printf("write:%s\n",wrtBuffer ); 
        write(udpRcvFd,wrtBuffer,strlen(wrtBuffer));*/

    }
    close(udpRcvFd);
    close(udpSockFd);








    close(sockFd);
    return 0;
}


void handle_alarm( int sig ) {

    //printf("ularmed!\n");
    dequeue();
   
}

void SIGIOHandler(int signalType)
{
    struct sockaddr_in sndAddr;             /* Address of sender */
    int sndAdLn = sizeof(sndAddr);          
    int recSz;                  
    char rcvBuffer[payloadSize+4];
    bzero(rcvBuffer,payloadSize+4);
    

    if ((recSz = recvfrom(udpSockFd, rcvBuffer, payloadSize+4, 0, (struct sockaddr *) &sndAddr, &sndAdLn)) < 0)
        {
            printf("Failed receive from client\n");
            exit(1);
        }

        char wrtBuffer[payloadSize];
        bzero(wrtBuffer,payloadSize);
        strncpy(wrtBuffer,rcvBuffer+4,recSz-4);

    int i ;
    for (i = 0; i < strlen(wrtBuffer); i++)
    {
        char value = wrtBuffer[i];
        //enqueue(value);
    }
        //printf("write:%s\n",wrtBuffer ); 
        //write(udpRcvFd,wrtBuffer,strlen(wrtBuffer));

}


void enqueue(char value){
    // wait if there is no space left:
    printf("spacesem is %d\n",spacesem );
    sem_wait( &spacesem );

    pthread_mutex_lock(&lock);
    semBuffer[ in ] = value;
    in = (in+1)%bufSz;

    pthread_mutex_unlock(&lock);
    //printf(" success enq, %c countsem %d spacesem %d ,in %d\n",value,&countsem,&spacesem,in );

    // increment the count of the number of items
    sem_post(&countsem);
}


void dequeue(){
    // Wait if there are no items in the buffer
    //printf("countsem is %d\n",countsem );

    sem_wait(&countsem);
    pthread_mutex_lock(&lock);
    char result = semBuffer[out];
    out = (out+1)%bufSz;

    pthread_mutex_unlock(&lock);
    printf(" countsem %d  \n", countsem);

    // Increment the count of the number of spaces
    sem_post(&spacesem);

}
