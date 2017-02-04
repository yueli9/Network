// simple UDP based wetalk app

#include <stdio.h>
#include <termios.h>

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
#define MAX_NAME_SZ 50


static void sig_alrm(int);
static int recvSockFd;                         // recvSockFd is for receiving from peer,make global for sigio
static int sndSockFd;
void SIGIOHandlerRcv(int signalType);      //  SIGIO handler
void SIGIOHandlerSnd(int signalType);      //  SIGIO handler
static int convAv = 0;           // before establish normal converservation

static int serverOutput =0;
static int clientOutput =0;

char readSvBuff[2048];
//bzero(readSvBuff,2048);
int rsbc =1;

char readClBuff[2048];
//bzero(readClBuff,2048);
int rcbc =0;



struct termios oldtermios;

int ttyraw(int fd)
{

    struct termios newtermios;
    if(tcgetattr(fd, &oldtermios) < 0)
        return(-1);
    newtermios = oldtermios;
 
//    newtermios.c_lflag &= ~(ICANON | ECHO );



/*
newtermios.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
                | INLCR | IGNCR | ICRNL | IXON);
newtermios.c_oflag &= ~OPOST;
newtermios.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
newtermios.c_cflag &= ~(CSIZE | PARENB);
newtermios.c_cflag |= CS8;

*/




    newtermios.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    newtermios.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    newtermios.c_cflag &= ~(CSIZE | PARENB);


    newtermios.c_cflag |= CS8;

    //newtermios.c_oflag &= ~(OPOST);

    newtermios.c_cc[VMIN] = 1;
    newtermios.c_cc[VTIME] = 0;


    if(tcsetattr(fd, TCSAFLUSH, &newtermios) < 0)
   // if(tcsetattr(fd, TCSANOW, &newtermios) < 0)
        return(-1);
    return(0);
}
    
     
int ttyreset(int fd)
{
    if(tcsetattr(fd, TCSAFLUSH, &oldtermios) < 0)
    //if(tcsetattr(fd, TCSANOW, &oldtermios) < 0)
        return(-1);

    return(0);
}




int main(int argc, char *argv[])
{

printf("To talk we another machine use: $IPAddress$PortNum\n");
//struct sigaction handler;           //Signal handling action definition 
char *token;                        // for split msg
int portNum;                        // record its own Port number


if (argc < 2) {
     printf("ERROR,lacking port number \n");
     exit(1);
 }

portNum = atoi(argv[1]);            // get port number


int serverMode = 1;             // serverMode receive msg first
int beforeNormal = 1;           // before establish normal converservation
int aWait = 1;                  // in cycle


while(aWait == 1)
{



struct sockaddr_in svaddr,ctaddr,rvAddr;

socklen_t rvSize = sizeof(rvAddr);


memset(&svaddr, 0, sizeof(svaddr));
svaddr.sin_family = AF_INET;
svaddr.sin_addr.s_addr = INADDR_ANY;
svaddr.sin_port = htons(portNum);

recvSockFd = socket(AF_INET,SOCK_DGRAM,0);                  // recerver's socket Fd

//sndSockFd = socket(AF_INET,SOCK_DGRAM,0);               // sndSockFd is for sending to peer

if(recvSockFd <0){
    printf("ERROR cereating socket\n");
    exit(1);
 }


if(bind(recvSockFd,(struct sockaddr *) &svaddr, sizeof(svaddr)) <0){
    printf("ERROR binding socket\n");
    exit(1);
}


    struct sigaction handler;           //Signal handling action definition 

    struct sockaddr_in sndAddr;     /* Address of sender */
    int sndAdLn = sizeof(sndAddr);          


    int imClient =0;
    int imServer =0;

    printf("?\n"); // wait for input
    char initMsg[50];
    bzero(initMsg,50);

    char sndMsg[10] = "wannatalk";

    char hostName[30];
    bzero(hostName,30);
    int hostPortNum =0;
    char Rbuf[3];



    fd_set rfds;

    int retval;

    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    FD_SET(recvSockFd, &rfds);


    sndSockFd = socket(AF_INET,SOCK_DGRAM,0);               // sndSockFd is for sending to peer


    retval = select(recvSockFd +1, &rfds, NULL, NULL, NULL);   // waiting for stdin and recvsockfd

    if(FD_ISSET(0, &rfds)){

    int conn =0;
    FD_ZERO(&rfds);


    signal(SIGALRM, sig_alrm);                  // register the alarm handler
    siginterrupt(SIGALRM, 1);


    while(conn ==0){            // keep on trying until success handshake

    printf("?\n");
    scanf("%s",initMsg);                                // take input msg, this part is client code

    if (strcmp(initMsg,"q")==0){
        printf("Input %s, exit from wetalk\n",initMsg );
        exit(1);
        }

    else{
        token = strtok(initMsg, "$");
        strcpy(hostName,token);
        token = strtok(NULL, "$");                      // get message from initMsg
        hostPortNum=atoi(token);

        struct hostent *server;                         
        server = gethostbyname(hostName);
        if (server == NULL){                           // get server name
            printf("ERROR no such hostname \n");
            exit(1);
        }       

        memset(&ctaddr, 0, sizeof(ctaddr));
        ctaddr.sin_family = AF_INET;
        bcopy((char *)server->h_addr, (char *)&ctaddr.sin_addr.s_addr, server->h_length);

        ctaddr.sin_port = htons(hostPortNum);


        //close(sndSockFd);
        if (connect(sndSockFd,(struct sockaddr *) &ctaddr,sizeof(ctaddr)) < 0) {       
            printf("ERROR creating socket\n");
            exit(1);
        }

        printf("success create socket to peer\n");

        // send handshake msg to peer
        int srl =sendto(sndSockFd,sndMsg,strlen(sndMsg) ,0,(struct sockaddr_in *)&ctaddr,sizeof(ctaddr));

        if(srl<0){
            printf("Failed to send to peer wetalk\n");
            exit(1);
        }

        alarm(7);                                       // alarm after 7 seconds,recvfrom timeout


        bzero(Rbuf,sizeof(Rbuf));
        int rvt = -1;
        rvt =recvfrom(sndSockFd,Rbuf,2,0,(struct sockaddr *)&rvAddr, &rvSize);
        if (rvt <0)
        {
        printf("ERROR reading stock or TimeOut\n");
        alarm(0);  // reset alarm
        //close(recvSockFd);
        //close(sndSockFd);
        continue;                                       // skip this handshake trial
        }
        else{
            //conn =1;
            printf("Reset Alarm\n");
            alarm(0);  // reset alarm
        }

        if (Rbuf[0] == 'O' && Rbuf[1] == 'K'){           // peer accept handshake! start conversation
            conn =1;
            convAv =1;
            imClient =1;
            continue;

        }
        else if (Rbuf[1] == 'O' && Rbuf[0] == 'K'){       // handshake rejected, skip this trial
            printf("| doesn't want to chat\n");
            continue;
        }
        }

    }

    }

    else if(FD_ISSET(recvSockFd, &rfds)){            // got msg from recsockfd
            //printf("I got msg\n");

            FD_ZERO(&rfds);

            int recSz;
            char rcvBuf[1024];
            bzero(rcvBuf,1024);

            int newc =0;
            while(newc ==0)
            {
                printf("?\n");
            if ((recSz = recvfrom(recvSockFd, rcvBuf, 1024, 0, (struct sockaddr *) &sndAddr, &sndAdLn)) < 0){
                    printf("recvfrom failed\n");
                    exit(1);
                }


            if (strcmp(rcvBuf,"wannatalk")==0)                          // got handshake request
            {
                char from_ip[50];
                inet_ntop(AF_INET, &sndAddr.sin_addr, from_ip, sizeof(from_ip));
                int ptn = ntohs(sndAddr.sin_port);
                printf("|chat request from %s : %d\n?\n",from_ip,ptn );
                char resb[51];
                bzero(resb,51);
                scanf("%s",resb);
                //fgets (resb, MAX_NAME_SZ, stdin);

                char feedb[2];
                bzero(feedb,2);

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

                int febl =sendto(recvSockFd,feedb,strlen(feedb) ,0,(struct sockaddr_in *)&sndAddr,sizeof(sndAddr));

                if(febl <0){
                    printf("Failed sendback \n");
                    exit(1);
                }

                if(strcmp("OK",feedb) == 0){                // agree to handshake
                    printf("Conversation Established\n");
                    convAv = 1;
                    imServer=1;   
                    newc=1;                              // mark server(the one who received request)
                    continue;
                    }

                if(strcmp("KO",feedb) == 0)                 // refuse to handshake
                    {

                        continue;}
            }
        }

            }



    if(convAv == 1 && imClient == 1){                                // start the formal conversation

        handler.sa_handler = SIGIOHandlerSnd;
        if (sigfillset(&handler.sa_mask) < 0) 
            {printf("sigfillset() failed");
            exit(1);}

        handler.sa_flags = 0;                       // no flags

        if (sigaction(SIGIO, &handler, 0) < 0)
            {printf("sigaction() failed for SIGIO");
            exit(1);}

           if (fcntl( sndSockFd , F_SETOWN, getpid()) < 0)
            {printf("Unable to set process owner to us");
            exit(1);}

        if (fcntl(sndSockFd, F_SETFL, O_NONBLOCK | FASYNC) < 0)
            {printf("Unable to put client sock into non-blocking/async mode");
            exit(1);}

        bzero(readClBuff,2048);


        printf(">\n");
        while(convAv==1){               // conversation 
            
            if(rcbc!=0)
                printf(">\n");

            
            char inputMsg[50];
            char transMsg[51];
            bzero(inputMsg,50);
            bzero(transMsg,51);

            if(ttyraw(0) < 0)
                {
                fprintf(stderr,"Can't go to raw mode.\n");
                exit(1);
            }
            //printf("raw mode\n");

            char c;
            rcbc =1;

            int i =strlen(readClBuff);

            while( (read(0, &c, 1) == 1)&& i<2048)
            {
                //printf( "%c\n\r", c);
                printf( "%c", c);
                //putchar(c);
                rcbc =0;
                
                i++;
                if( (c &= 255) ==015) 
                {   rcbc =1;
                    break;
                }
                else{
                    readClBuff[i-1] =c;
                }
            } 

            if(ttyreset(0) < 0)
                {
                    fprintf(stderr, "Cannot reset terminal!\n");
                    exit(-1);
                }
            
            //printf("cont:%s leng %d\n", readClBuff,strlen(readClBuff));


            if(strlen(readClBuff)==0)
                continue;

            
            if(rcbc ==1)
            {
            strncpy(inputMsg, readClBuff,49);
            inputMsg[49] = 0; //null terminate destination
            bzero(readClBuff,2048);
            //rcbc=0;

            sprintf(transMsg,"D%s",inputMsg);
            if (strcmp(inputMsg,"e") ==0 )
            {
            convAv = 0;
            transMsg[1]='E';

            }

            if(sendto(sndSockFd,transMsg,strlen(transMsg) ,0,(struct sockaddr_in *)&ctaddr,sizeof(ctaddr))<0){
            printf("Failed to send to peer wetalk\n");
            //exit(1);
            }
            else{
                bzero(transMsg,51);
            }

            }
            else
                continue;


        }

        handler.sa_handler = SIG_DFL;
        close(sndSockFd);
        close(recvSockFd);
        printf("Next round\n");

        continue; // signal handler will make convAv ==0;

    }


    if(convAv == 1 && imServer ==1){                                // start the formal conversation

        handler.sa_handler = SIGIOHandlerRcv;
        if (sigfillset(&handler.sa_mask) < 0) 
            {printf("sigfillset() failed");
            exit(1);}

        handler.sa_flags = 0;                       // no flags

        if (sigaction(SIGIO, &handler, 0) < 0)
            {printf("sigaction() failed for SIGIO");
            exit(1);}

       if (fcntl(recvSockFd, F_SETOWN, getpid()) < 0)
           {printf("Unable to set process owner to us");
            exit(1);}

        if (fcntl(recvSockFd, F_SETFL, O_NONBLOCK | FASYNC) < 0)
            {printf("Unable to put client sock into non-blocking/async mode");
            exit(1);}

        bzero(readClBuff,2048);
        printf(">\n");

        while(convAv==1){               // conversation 

            if(rcbc!=0)
                printf(">\n");

            char inputMsgFB[50];
            char transMsgFB[51];
            bzero(inputMsgFB,50);
            bzero(transMsgFB,51);

 
            if(ttyraw(0) < 0)
            {
                fprintf(stderr,"Can't go to raw mode.\n");
                exit(1);
            }

            char c;
            int i =strlen(readClBuff);
            rcbc =1;
             while( (read(0, &c, 1) == 1)&& i<2048)
            {

                printf( "%c\n\r", c);
                rcbc =0;
                
                i++;
                if( (c &= 255) ==015) 
                {   rcbc =1;
                    //readClBuff[i] ='\0';        // replace enter with NULL
                    break;
                }
                else{
                    //printf( "%c\n\r", c);
                    readClBuff[i-1] =c;
                }
            } 
            

            ttyreset(0);
            //printf("cont:%s leng %d\n", readClBuff,strlen(readClBuff));


            if(strlen(readClBuff)==0)
                continue;
            
            if(rcbc ==1)
            {
            //rcbc =0;
            strncpy(inputMsgFB, readClBuff,49);
            inputMsgFB[49] = 0; //null terminate destination
            bzero(readClBuff,2048);
            sprintf(transMsgFB,"D%s",inputMsgFB);

            if (strcmp(inputMsgFB,"e") ==0 )
            {
            convAv = 0;
            transMsgFB[1]='E';
            }

            if(sendto(recvSockFd,transMsgFB,strlen(transMsgFB) ,0,(struct sockaddr_in *)&sndAddr,sizeof(sndAddr))<0){
            printf("Failed to send to peer wetalk\n");
            //exit(1);
            }
              else{
            bzero(transMsgFB,51);
            }
        }
        else
            continue;
            

        }
        handler.sa_handler = SIG_DFL;
        close(sndSockFd);
        close(recvSockFd);

        printf("next round\n");
        continue; 

    }



}

close(sndSockFd);
close(recvSockFd);
return 0;
}


static void sig_alrm(int signo){
    printf("TimeOut\n");
    return;                     /* just interrupt the recvfrom() */
}




 void SIGIOHandlerRcv(int signalType)
{
    struct sockaddr_in sndAddr;     /* Address of sender */
    int sndAdLn = sizeof(sndAddr);          
    int recSz;                  
    char rcvBuf[1024];       

    do
    {
        bzero(rcvBuf,1024);
        if ((recSz = recvfrom(recvSockFd, rcvBuf, 1024, 0, (struct sockaddr *) &sndAddr, &sndAdLn)) < 0)
        {
            if (errno != EWOULDBLOCK)  
            {
                printf("sh,recvfrom failed\n");
                exit(1);
                continue;
            }
        }
        else if(recSz>1)
        {
            if (strcmp(rcvBuf,"DE") == 0)
            {
            convAv = 0;     // end the conversation
            }

            else if (rcvBuf[0]=='D')
            {
            rcvBuf[0]='|';
            printf("%s\n\r",rcvBuf);
            //printf(">\n\r");
             if(rcbc==0){
            printf(">\n\r");

            printf("incomplete %s\n\r", readClBuff);
            }

            }

        }
    }  while (recSz >= 0);

    return;
}



 void SIGIOHandlerSnd(int signalType)

{

    struct sockaddr_in sndAddr;     /* Address of sender */
    int sndAdLn = sizeof(sndAddr);          
    int recSz;                  
    char rcvBuf[1024];       


    do
    {
        bzero(rcvBuf,1024);

        if ((recSz = recvfrom(sndSockFd, rcvBuf, 1024, 0, (struct sockaddr *) &sndAddr, &sndAdLn)) < 0)
        {

            //printf("failed recvfrom %d\n",recSz);
            /* Only acceptable error: recvfrom() would have blocked */
            if (errno != EWOULDBLOCK)  
            {
                printf("sigh,recvfrom failed\n");
                exit(1);
            }
        }
        else if(recSz >1)
        {

            if (strcmp(rcvBuf,"DE") == 0)
            {
            convAv = 0;     // end the conversation
             printf("Snd convAv :%d\n",convAv );

            }

            else if (rcvBuf[0]=='D')
            {
            rcvBuf[0]='|';
            printf("%s\n\r",rcvBuf);
            //printf(">\n\r");

            if(rcbc==0){
            printf(">\n\r");
            printf("incomplete %s\n\r", readClBuff);
            }


            }

        }
    }  while (recSz >= 0);
    return;


}



