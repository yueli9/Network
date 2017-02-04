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
//pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static int countsem;
static int spacesem;

char semBuffer[5000000];

void SIGIOHandler(int signalType);      //  SIGIO handler
void handle_alarm(int sig);             //  SIGIO handler
void myplot();
void enqueue(char value);
char dequeue();

static int udpSockFd;
static int payloadSize;
struct sockaddr_in udpServerAddr;
int gamma;
int targetBuf;
static int firstAlarm = 1;
int audioFd;

int intervalCount = 0;
int intervalTotal = 0;

int qtArr[50000];
int timeArr[50000];
static int t =0;
char logfileC[24];
 int playbackDel;
int interval ;
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
        printf("./audiolisten 128.10.3.57 57666 52833 250  2500 33 40000 20000 logC tmp/pp.au\n");
        exit(0);
    }

    char serverIP[20];
    bzero(serverIP,20);
    strcpy(serverIP,argv[1]);
    int serverTcpPort = atoi(argv[2]);                
    int clientUdpPort = atoi(argv[3]);
    payloadSize = atoi(argv[4]);
    //float fplaybackDel = (float)atof(argv[5]); 
    playbackDel = atoi(argv[5])*1000; // 1ms = 1000 usec
    gamma = atoi(argv[6]);
    bufSz = atoi(argv[7]);
    targetBuf = atoi(argv[8]);
    
    bzero(logfileC,24);
    strcpy(logfileC,argv[9]);
    char fileName[240];
    bzero(fileName,240);
    strcpy(fileName,argv[10]);


    //bzero(semBuffer,500000);

    //sem_init(&countsem, 0, 0);
    //sem_init(&spacesem, 0, bufSz);
    countsem = 0;
    spacesem = bufSz;



    
    audioFd=open("/dev/audio", O_WRONLY, S_IRUSR | S_IWUSR);
    //audioFd=open("audio", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);

    if (audioFd<0)
    {

        printf("Failed open audio\n");
        exit(1);
    }


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


    //printf("received:%s\n",Rbuf);

    char *pch;
    int rcvUDPport;

    pch = strtok (Rbuf," ");
    pch = strtok (NULL," ");                    // pch is portnumber
    rcvUDPport = atoi(pch);
    printf("Received portnumber %d\n",rcvUDPport );


    struct sockaddr_in udpClientAddr;
    int udpServerLen ;
    memset(&udpServerAddr, 0, sizeof(udpServerAddr));
    memset(&udpClientAddr, 0, sizeof(udpClientAddr));


    udpSockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSockFd <0){       
        printf("ERROR openning udpSockFd");
        exit(1);
    }


    udpServerAddr.sin_family = AF_INET;
    udpServerAddr.sin_addr.s_addr = inet_addr(serverIP);                          // set up server ip and port
    udpServerAddr.sin_port = htons(rcvUDPport);



    udpClientAddr.sin_family = AF_INET;
    udpClientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    udpClientAddr.sin_port = htons(clientUdpPort);                                  // bind udpclientport to udp client

/* 
    if (bind(udpSockFd,(struct sockaddr *) &udpClientAddr,sizeof(udpClientAddr))<0)
    {
        printf("Failed to bind clientUdpPort!\n");
        exit(1);
    }


    if (connect(udpSockFd,(struct sockaddr *) &udpServerAddr,sizeof(udpServerAddr)) < 0) {       
        printf("ERROR creating udpSockFd\n");
        exit(1);
    }

*/
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

    interval = (int) (1 /(float) gamma *1000000);     //?????
    printf("interval is %d\n",interval );


   // exit(1);
    
    //intervalTotal = 1000000000 /interval;
    //interval = 10;
    //exit(1);

    printf("Before Fist alarm 2s: %d \n",time(NULL) );

    //alarm(playbackDel/1000000);
    //sleep(20);
    ualarm(interval,interval);  //2*900000+570000 ns= 2.55s

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

    printf("ularmed!, Qt is %d\n",countsem);





    if ( playbackDel >0)
    {

        playbackDel = playbackDel - interval;

        //firstAlarm = 0; // ignore the first alarm
        printf("Fist alarm 2s: %d \n",time(NULL) );
    }
    

    else{



    qtArr[t] = countsem;
    timeArr[t] = time(NULL);
    t++;

    int i ;
    char deqBuf[payloadSize];
    bzero(deqBuf,payloadSize);
    for (i = 0; i < payloadSize; i++)       // each alarm should dequeue payloadSize bytes
    {
        deqBuf[i] = dequeue();
    }

    int wr  =write(audioFd,deqBuf,strlen(deqBuf));
    if (wr<0)

   {

        printf("Failed write audio\n");
                //exit(1);

    }

    else{
        printf("Sucee writing audio\n");
    }

    char sendBuffer[50];
    bzero(sendBuffer,50);
    sprintf(sendBuffer,"Q %d %d %d",countsem,targetBuf,gamma);  // contsem works as Q(t)


    int srl =sendto(udpSockFd,sendBuffer,strlen(sendBuffer) ,0,(struct sockaddr_in *)&udpServerAddr,sizeof(udpServerAddr));

    if(srl <= 0){
        printf("Failed to send back to server,connection closed\n");



        FILE *fp;

        fp = fopen(logfileC, "w+");
        fprintf(fp, "This is testing for fprintf...\n");
        
        int l = 0;
        for(l =0 ;l<t;l++)
        {
            char bbuf[50] = {0}; 

            sprintf(bbuf, "%d %d\n", timeArr[l],qtArr[l]);

            fputs(bbuf, fp);
        }
        fclose(fp);
        myplot();








        exit(1);
    }
        printf("ularm end! %d \n",srl);

    }

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
            printf("Failed receive from server\n");

}

if (rcvBuffer[0] == 'C')

{
        FILE *fp;

        fp = fopen(logfileC, "w+");
        fprintf(fp, "This is testing for fprintf...\n");
        
        int l = 0;
        for(l =0 ;l<t;l++)
        {
            char bbuf[50] = {0}; 

            sprintf(bbuf, "%d %d\n", timeArr[l],qtArr[l]);

            fputs(bbuf, fp);
        }
        fclose(fp);

        myplot();


            exit(1);


        }





    qtArr[t] = countsem;
    timeArr[t] = time(NULL);
    t++;

        char wrtBuffer[payloadSize];
        bzero(wrtBuffer,payloadSize);
        strncpy(wrtBuffer,rcvBuffer+4,recSz-4);
    printf("received:%s\n",rcvBuffer ); 


    int i ;
    for (i = 0; i < strlen(wrtBuffer); i++)
    {
        char value = wrtBuffer[i];
        enqueue(value);
    }
        //printf("write:%s\n",wrtBuffer ); 
        //write(udpRcvFd,wrtBuffer,strlen(wrtBuffer));

    //int tau = 1000;
    char sendBuffer[1024];
    bzero(sendBuffer,1024);
    sprintf(sendBuffer,"Q %d %d %d",countsem,targetBuf,gamma);
    int srl =sendto(udpSockFd,sendBuffer,strlen(sendBuffer) ,0,(struct sockaddr_in *)&udpServerAddr,sizeof(udpServerAddr));

    if(srl < 0){
        printf("Failed to send back to server\n");
        exit(1);
    }
    //printf("SEND banck %s \n",sendBuffer );
    //exit(1);

}


void enqueue(char value){
    // wait if there is no space left:
    //printf("spacesem is %d\n",spacesem );
    //sem_wait( &spacesem );

    if (spacesem >0)
    {
        semBuffer[ in ] = value;
        in = (in+1)%bufSz;
        spacesem--;
        countsem++;
        //printf(" success enq, %c countsem %d spacesem %d ,in %d\n",value,countsem,spacesem,in );
    }
    else
    {
        //printf("Buffer is FULL!\n");
        //exit(1);
    }
 




}


char dequeue(){


    
    if (countsem>0)
    {
        char result = semBuffer[out];
        out = (out+1)%bufSz;
        countsem--;
        spacesem++;
        //printf(" success dequeue, %c countsem %d spacesem %d ,out %d\n",result,countsem,spacesem,out );

        return result;
    }
       else
    {
        printf("Buffer is EMPTY!\n");
                //exit(1);

        return '\0';
    }
    //pthread_mutex_lock(&lock);


    //pthread_mutex_unlock(&lock);
    //printf(" countsem %d  \n", countsem);

    // Increment the count of the number of spaces
    //sem_post(&spacesem);

}


void myplot(){

    char * commandsForGnuplot[] = {"set title \"Time-qt\"", "plot 'data1.temp2'"};
   
    int k =0;

    int allv = 1;
    int ttm = timeArr[0];

   for(k=0;k<t;k++)
    {
        if(timeArr[k] > ttm){
            allv++; 
            ttm = timeArr[k] ;  

        }

    }

    int xvals[allv] ;
    int yvals[allv]  ;


    xvals[0] = 0;
    yvals[0] = 0;
    int ct = 1;
    int tm = timeArr[0];
    int tm0 = timeArr[0];
    k = 0;
    printf("Start assign\n");
    for(k=0;k<t;k++)
    {
        if(timeArr[k] > tm){
            tm = timeArr[k] ;
            xvals[ct] =timeArr[k]-tm0;
            yvals[ct] = qtArr[k];
            ct++;
            printf("ct % d x %d y %d, qt %d \n",ct, xvals[ct],yvals[ct],qtArr[k]);
        }

    }

    int NUM_COMMANDS = 2;

    FILE * temp = fopen("data1.temp2", "w");

    FILE * gnuplotPipe = popen ("gnuplot -persistent2", "w");
    int i;
    for (i=0; i < allv; i++)
    {
    fprintf(temp, "%d %d \n", xvals[i], yvals[i]); //Write the data to a temporary file
    }

    for (i=0; i < NUM_COMMANDS; i++)
    {
    fprintf(gnuplotPipe, "%s \n", commandsForGnuplot[i]); //Send commands to gnuplot one by one.
    }
    

    }