#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <fcntl.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "mylab6.h"

#define MAXPACKSZ 5001
#define WINSZ 4
#define CHECKTIME 100

int sldWin_size[WINSZ]; 
char sldWin[WINSZ][MAXPACKSZ];   


int prCount = 0;
int count;   
int newSocket;  
struct sockaddr_in clntAddrStore;   


bool eofCheck = false;

bool iniCheck = true;

void alrmHandl(int sig);

int createSocket(int portNum);

int getMin(int a, int b);

int lossNum ;

void main(int argc, char * argv[]){
  //argument check
  if(argc != 5){
        printf("ERROR turboserver portnumber secretkey configfile.dat lossnum\n");
        exit(1);
    }

    if(strlen(argv[2])<10 || strlen(argv[2])>20){                                           // check secrete key
         printf("ERROR,security key length should be from 10 to 20\n");
         exit(1);
    }

    lossNum = atoi(argv[4]);

    char bufBlockSize[10];
    int blocksize=0;
    int bsfd = open(argv[3],O_RDONLY);

    int nbs = read(bsfd,bufBlockSize,sizeof(bufBlockSize)-1);                     // receive msg from client
    if(nbs >=16)
    {
        printf("blocksize should not exced 16 characters.\n");                   // check filename size
        exit(1);
    }

    bufBlockSize[nbs] ='\0';
    blocksize = atoi(bufBlockSize);                                             // get blocksize
    printf("blocksize is %d \n",blocksize);

    char secKey[20];
    bzero(secKey,20);
    strcpy(secKey,argv[2]);                                             // get secretekey

    
    struct timeval iniTime, finTime;

    // handle time out
    struct sigaction sact_t;
    sigemptyset( &sact_t.sa_mask );
    sact_t.sa_handler = alrmHandl;
    sact_t.sa_flags = SA_RESTART;

    sigaction(SIGALRM, &sact_t, NULL);
   
    // initilized window
    int t;
    for(t=0;t<WINSZ;t++)
      {
        bzero(sldWin[t],sizeof(sldWin[t]));
      }

    int i;

    // create socker
    int sockfd = createSocket(atoi(argv[1]));
    
    int prCount = -1;
    int skCheck = 0;
    while(1){

      int cltAddrlen = sizeof(clntAddrStore);
      char inibuff[1000];
      bzero(inibuff,1000);

      int n = recvfrom(sockfd, inibuff, sizeof(inibuff), 0, (struct sockaddr *)&clntAddrStore,&cltAddrlen);
      if (n <0){
      printf("Failed recvfrom: %d\n", n);
      }


      char csecKey[20];
      char *token;
      printf("inibuff %s\n ",inibuff);
      token = strtok(inibuff, "$");
      strcpy(csecKey,token);                         // get secrete key from buf

      if(strcmp(csecKey,secKey) != 0){               // check if secrete key match
        printf("secretkey does not match \n");
        continue;
      }

      token = strtok(NULL, "$");                    // get file name from buf
      printf("buf after: %s\n ",inibuff);  


      char filename[50];
      sprintf(filename, "%s%s", "./filedeposit/", token); 


      if(access(filename, F_OK | R_OK) != 0){   //check the file path
        printf("File does not exit\n");
        printf("Make sure to put file in filedeposit folder!!\n");
        continue;
      }


        int status;
        unsigned char buff[2048];
        bzero(buff,2048);


        int k = fork();
          // child process
        if(k == 0){
          gettimeofday(&iniTime, NULL);  //get init time

          newSocket =  createSocket((rand()%65535)+1);

          int fp, rd;
          fp = open(filename, O_RDONLY);
          if (fp <0){
            printf("Could not open the File\n");
            break;
          }

          unsigned char readBuff[blocksize+1];
          count = 0;
          int mycount = 0;
          

          while(1){ 
    
          if ((rd=read(fp,readBuff+1,blocksize)) == 0 && mycount > count){ // read file
            break;
          }
            if(rd==0){
             // printf("EOF \n");
            }
            else{
              readBuff[0] = count;
              memcpy(sldWin[count%WINSZ], readBuff, rd+1);
              sldWin_size[count%WINSZ] = rd+1;
            }

            while(1){

              if (count < WINSZ-1 && rd != 0)
              {
                break;
              }
              int i;

              int tmpm = getMin(WINSZ,count);
              i = 0;
              while(iniCheck == true&&i<tmpm){
                dropsendto(newSocket,sldWin[i],sldWin_size[i],0,(struct sockaddr *)&clntAddrStore,sizeof(clntAddrStore),1000,lossNum); 
                i++;
              }

              ualarm(CHECKTIME,0);

              if(skCheck > 0){skCheck --;break;}


              iniCheck = false;

              // check ACk
              if(buff[0] == 'P'){
                buff[0] = 0;
                int tmp = (int)buff[1];

                if(prCount == tmp && 1){ 
                 // printf("equal?\n");
                  prCount = tmp;
                }
                else{
                 if(tmp - prCount < 0){
                    skCheck = tmp - prCount+256;

                    mycount += 256+tmp-prCount;
                  }
                  else{
                    skCheck = tmp - prCount;

                    mycount += tmp-prCount ;
                  }
                  prCount = tmp;

                }

              }


              if(buff[0] == 'N'){
                buff[0] = 0;
                int msCheck = (int)buff[1];
                if(msCheck >= count){

                }else{
                  msCheck = msCheck%WINSZ;
                  dropsendto(newSocket,sldWin[msCheck],sldWin_size[msCheck],0,(struct sockaddr *)&clntAddrStore,sizeof(clntAddrStore),1000,lossNum);
                }
              }


              if(mycount != count  ){

              }

              else{
              if(rd != 0 ){
                              }
              else{

                  gettimeofday(&finTime, NULL); 
                  eofCheck = true;
                  char *eof = "EOF";
                  sendto(newSocket,eof,4,0,(struct sockaddr *)&clntAddrStore,sizeof(clntAddrStore)); 


                  }
                }


              //check EOF, wait for ACk
              if(eofCheck == false){
                if(recvfrom(newSocket, buff, sizeof(buff), 0, (struct sockaddr *)&clntAddrStore,&cltAddrlen)<0)
                {printf("ERROR receiving\n");}
              }
              else{
                  unsigned long totalt =  (finTime.tv_sec* 1000000  + finTime.tv_usec )      
                                         - (iniTime.tv_sec *1000000  + iniTime.tv_usec);
                  printf("\n completion time %ld usec\n ",totalt);
                  mycount = count + 1;
                  break;
                }

            }

            if(rd!=0){
              count++;
            }
            
          }
          close(newSocket);
          close(fp);
          exit(1);
        }
    }   
}


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

void alrmHandl(int sig){

  if(iniCheck != true)
  {
    //printf("First time only\n");
  }
  else{
    int i;
    if(count != prCount){
      //printf("Count does no match\n");
    }
    else{

    int tmpm = getMin(WINSZ-1,count);
    while (i<= tmpm)
    {
      dropsendto(newSocket,sldWin[i],sldWin_size[i],0,(struct sockaddr *)&clntAddrStore,sizeof(clntAddrStore),200,1);      
      i++;
    }

  }
  prCount = count;
  }
  ualarm(CHECKTIME);
  
}

int getMin(int a, int b){
    if (a >b)
    {
      return a;
    }
    else
      return b;
}
