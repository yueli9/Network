/* mylab6.h */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netdb.h> 
#include <stdbool.h>
#include "mylab6.h"
//#include <mylab6.h>

//const int WINSZ = 4;
//int PACKSZ = 1000;
//int CHECKTIME = 100;

#define WINSZ 4
#define MAXPACKSZ 5001
#define CHECKTIME 100
//define window and some secondary window for size storage


//the window starting index
struct sockaddr_in svAddStor;
bool lockSld = true;
int shift = 0;
int sockFd;
int send_packetNumer = 0;


void alarmHandler(int sig);

char sldWin[WINSZ][MAXPACKSZ];
int sldCount[WINSZ];
int sldSize[WINSZ];


void main(int argc, char * argv[]){

    if (argc < 6) {
        printf("turboclient hostname portnumber secretkey filename configfile.dat \n");  // check arguments number
        exit(0);
    }

    int i;
    

    struct sockaddr_in servAddr;
    
    socklen_t sendsize = sizeof(svAddStor);
    bzero(&svAddStor, sizeof(svAddStor));   //clear


    //socklen_t len;
    char buff[1024];
    bzero(buff,1024);
    char recv_buff[1024];
    bzero(recv_buff,1024);
    //define socket
    sockFd = socket(AF_INET, SOCK_DGRAM, 0);
    struct hostent *server;
    server = gethostbyname(argv[1]);

    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&servAddr.sin_addr.s_addr, server->h_length);
    servAddr.sin_port = htons(atoi(argv[2]));

    // check secrete key
    if(strlen(argv[3])<10 || strlen(argv[3])>20){
      printf("ERROR,security key length should be from 10 to 20\n");
      exit(1);    
    }

    char secKey[20];
    bzero(secKey,20);
    strcpy(secKey,argv[3]);



    // get the file name and do the check
    char fileName[240];
    bzero(fileName,240);
    strcpy(fileName,argv[4]);                               
    int fi =0;
    for(fi = 0;fi<strlen(fileName);fi++){
      if(fileName[fi] == '/'){                            // make sure not '/' exist
        printf("could not include '/'' in fileName");
      }
    }

    if( access( fileName, F_OK ) != -1 ) {                  // check if file already exist
        printf("File already exist,delete it first.\n");
        //exit(1);
    } 

    char bufBlockSize[50];
    int bsfd = open(argv[5],O_RDONLY);                       //get buffer size
    int nbs = read(bsfd,bufBlockSize,sizeof(bufBlockSize)-1);

    if(nbs >=16){
    printf("blocksize should not exced 16 characters.\n");
    exit(1);
    }

    bufBlockSize[nbs] ='\0';
    int blocksize = atoi(bufBlockSize);
    printf("blocksize is %d \n",blocksize);

    char iniBuf[1000];
    bzero(iniBuf,1000);
    sprintf(iniBuf, "$%s$%s", secKey, fileName);               // create the message to send

    if (sendto(sockFd,iniBuf,sizeof(iniBuf),0,(struct sockaddr *)&servAddr,sizeof(servAddr)) <0)
    {
      printf("ERROR send to server \n");
      exit(1);
    }

    // regist for signal alarm handler
    struct sigaction sigAct;
    sigemptyset( &sigAct.sa_mask );
    sigAct.sa_handler = alarmHandler;
    sigAct.sa_flags = SA_RESTART;
    sigaction(SIGALRM, &sigAct, NULL);

    
    unsigned char rcvBuff[5000];
    bzero(rcvBuff,5000);

    char tmpfm[100];
    bzero(tmpfm,100);
    sprintf(tmpfm,"./tmp/%s",fileName);
    int newFd = open(tmpfm, O_WRONLY|O_CREAT|O_TRUNC, 0644); // open if not exist create
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    int filesz = 0;
    int t;
    for(t=0;t<WINSZ;t++)
      {sldCount[t] = -2;    
        bzero(sldWin[t],sizeof(sldWin[t]));
      }

    // keep checking for new msg
    while (1)  
    {
      int n =0;
      if ((n=recvfrom(sockFd,rcvBuff,blocksize+1,0,(struct sockaddr*)&svAddStor, &sendsize)) <0)
      {
        break;
      }

      //printf("Recv Buff : %s\n",rcvBuff );
      //no check while sliding window
      lockSld = true;
      if(strcmp(rcvBuff, "EOF") != 0){    //check EOF

        int packetNum = (int) rcvBuff[0];     // get the packet number
        int sldPos = packetNum%WINSZ;   // get its postition in sliding window
        sldCount[sldPos] = packetNum;
        sldSize[sldPos] = n-1;
        memcpy(sldWin[sldPos],rcvBuff+1,n-1);

        //strcpy(sldWin[sldPos], rcvBuff+1);
        bzero(rcvBuff,5000);

   
        // check first slide availability        
        if( shift%256 == packetNum ){
          // sendback postive ack
          unsigned char pAck[2];
          pAck[0] = 'P';
          pAck[1] = packetNum; 
          dropsendto(sockFd,pAck,2,0,(struct sockaddr *)&svAddStor,sizeof(svAddStor),1000,1);

          ualarm(CHECKTIME,0);
          int tmp = 0;


          i = 0;
          while(i < WINSZ){
              if(sldCount[(shift+i)%WINSZ] != (shift+i)%256){
                break;

              }
              else{

                int writeSize = sldSize[(shift+i)%WINSZ];
                filesz += writeSize;
                write(newFd, sldWin[(shift+i)%WINSZ], writeSize);
                tmp++;
              }
            i++;
          }


          shift += tmp;         
          }

          }

        else{ 

        printf("EOF\n");


        gettimeofday(&end, NULL);           


        unsigned long totalt =  (end.tv_sec* 1000000  + end.tv_usec )      // calculate time spend
          - (start.tv_sec *1000000  + start.tv_usec);

        printf("\n completion time %ld usec, bytes transferred %d, reliable throughput %d mbps \n ",totalt,filesz,filesz/totalt);

        close(newFd);
        break;
      }

      lockSld = false;
    }
    
    close(newFd);

}




//timeout handler resend lost ACK packets
void alarmHandler(int sig){
    if (lockSld == false){
      unsigned char ACK[2];
      ACK[0] = 'P';
      ACK[1] = shift%256-1; 
      dropsendto(sockFd,ACK,2,0,(struct sockaddr *)&svAddStor,sizeof(svAddStor),1000,1);
      int i = 0;
      while(i<WINSZ){

        if(sldCount[i] <0 || sldCount[(shift+i)%WINSZ] != (shift+i)%256){
          ACK[0] = 'N';
          ACK[1] = shift+i;
          dropsendto(sockFd,ACK,2,0,(struct sockaddr *)&svAddStor,sizeof(svAddStor),1000,1);
        }
        i++;
      }

    }
    else{
        ualarm(CHECKTIME,0);
        return;
    }

    ualarm(CHECKTIME,0);

}