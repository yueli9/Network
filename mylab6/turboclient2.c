#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <netdb.h> 
#include <stdbool.h>


//#define debug 1
//SLIDING WINDOW size and Packet size
#define WINDOW_SIZE 4
#define PACKET_SIZE 1000
//#define TIMEOUT 900000

//timeout value for not receiving any SERVER message
/*this value need be tuned for different network situation, if network loses a lot of packets set this be small 
*and opposite otherwise
*e.g. 200 for non loss
*     50 for 50% loss
*/
#define TIMEOUT 100
//define window and some secondary window for size storage
char slideWindow[WINDOW_SIZE][PACKET_SIZE+1];
int slideSize[WINDOW_SIZE];
int slideCount[WINDOW_SIZE];

//the window starting index
int windSd = 0;
int sockFd;

//define the socket as global varaiable
struct sockaddr_in servStore;
struct itimerval timer;

//mutex lock for receive
bool lockWind = true;
int print_max = 1;
//int send_packNumer = 0;
//mysend based on probability
ssize_t mysendto(int sockFd, const void *buf, size_t len, int flags,
                      const struct sockaddr *dest_addr, socklen_t addrlen, int totalnum, int lossnum){
              int res;    

              printf("SENDTO %s\n",buf);
              //send_packNumer ++;
            //printf("==%d==",send_packNumer);
              float rate = (double)lossnum/totalnum;    
              float ran = ((double)rand() / (double)(RAND_MAX)); 
              //printf("%f ",ran);          
              if(ran <= 0.9){
                res = sendto(sockFd, buf, len, flags, dest_addr, addrlen); 
                //printf("==%c,%c==\n",(char)buf[0], (char)buf[1]);
              }else res = 0;
              return  res;        
}
//timeout handler resend lost ACK packets
void myhandler(int sig){
            //define timeour
            struct itimerval tout_val;
  
            tout_val.it_interval.tv_sec = 0;
            tout_val.it_interval.tv_usec = 0;
            tout_val.it_value.tv_sec = 0; 
            tout_val.it_value.tv_usec = TIMEOUT;
            
            //printf("%d;;;;",lockWind);
            //mutex lock check, cannot do sig when is running during moving sliding window
            if(lockWind == true){

              setitimer(ITIMER_REAL, &tout_val,0);
              //ualarm(TIMEOUT,0);

              return;}
            int i;
            unsigned char PACK[2];
            PACK[0] = '+';
            PACK[1] = windSd%256-1; 
            //resend the lost postive ACK
            mysendto(sockFd,PACK,2,0,(struct sockaddr *)&servStore,sizeof(servStore),3,1);
            
            //Check which part of the window is not fill and send the negative ACK
            for(i=0;i<WINDOW_SIZE;i++){
                
                //only when index match the packet number stored
                if(slideCount[i] == -1 || slideCount[(windSd+i)%WINDOW_SIZE] != (windSd+i)%256)             {
                    unsigned char NACK[2];
                    NACK[0] = '-';
                    NACK[1] = windSd+i;
                    mysendto(sockFd,PACK,2,0,(struct sockaddr *)&servStore,sizeof(servStore),3,1);
                }
            }
            setitimer(ITIMER_REAL, &tout_val,0);
            //ualarm(TIMEOUT,0);


}

void main(int argc, char * argv[]){

    if (argc < 6) {
        printf("turboclient hostname portnumber secretkey filename configfile.dat \n");  // check arguments number
        exit(0);
        }

        struct timeval send_time, recv_time;
        struct sigaction sact;
        sigemptyset( &sact.sa_mask );
        sact.sa_flags = SA_RESTART;
        sact.sa_handler = myhandler;
        sigaction(SIGALRM, &sact, NULL);
        
        int i,n;
        //init the window buffer to store each packet size
        for(i=0;i<WINDOW_SIZE;i++)slideCount[i] = -1;      
        struct sockaddr_in client_addr;
        bzero(&client_addr, sizeof(client_addr));

        
        socklen_t sStroeSize = sizeof(servStore);
        bzero(&servStore, sizeof(servStore));     //clear
        //struct scokaddr_in _sin;
        socklen_t len;
        char recv_buff[1024];
        


        //define socket
        sockFd = socket(AF_INET, SOCK_DGRAM, 0);
        // get server address
        struct sockaddr_in svaddr;
        bzero(&svaddr, sizeof(svaddr));

        struct hostent *server;
        server = gethostbyname(argv[1]);
        if (server == NULL){                                    // get server name
          printf("ERROR no such hostname");
          exit(1);
        }
        int portNum = atoi(argv[2]);
        svaddr.sin_family = AF_INET;
        bcopy((char *)server->h_addr, (char *)&svaddr.sin_addr.s_addr, server->h_length);
        svaddr.sin_port = htons(portNum);


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
            exit(1);
        } 



        // get the block size
        char bufBlockSize[50];
        int bsfd = open(argv[5],O_RDONLY);                       //get buffer size
        int nbs = read(bsfd,bufBlockSize,sizeof(bufBlockSize)-1);

        if(nbs >=16)
        {
        printf("blocksize should not exced 16 characters.\n");
        exit(1);
        }

        bufBlockSize[nbs] ='\0';
        int blocksize = atoi(bufBlockSize);
        printf("blocksize is %d \n",blocksize);




        // send initial msg to server;
        char buff[1000];
        bzero(buff,1000);

        sprintf(buff, "$%s$%s", secKey, fileName);               // create the message to send

        int issize = sendto(sockFd,buff,sizeof(buff),0,(struct sockaddr *)&svaddr,sizeof(svaddr));
        if (issize<0)
        {
          printf("Failed send to server \n");
          exit(1);
        }


        //get time of the day
        gettimeofday(&send_time, NULL);


        char rcvBuff[5000];   // store reeived msg
        bzero(rcvBuff,5000);
        int fp;
        fp = open(fileName, O_WRONLY|O_CREAT|O_TRUNC, 0644);
        
        int packNum = 0;
        //int total = 0;
        //int prev = -1;
        
        //timeout sig init
       struct itimerval tout_val;
  
        tout_val.it_interval.tv_sec = 0;
        tout_val.it_interval.tv_usec = 0;
        tout_val.it_value.tv_sec = 0; 
        tout_val.it_value.tv_usec = TIMEOUT;
       

        while (1)    
        {
          if((n=recvfrom(sockFd,rcvBuff,PACKET_SIZE+1,0,(struct sockaddr*)&servStore, &sStroeSize)) <0) 
          {
            printf("ERROR receive \n");
            close(fp);
            break;

          }

          printf("%s\n",rcvBuff);

          // lock ,could not do send when sliding
          lockWind = true;
          if(strcmp("EOFEOF",rcvBuff) != 0){       // check it is NOT EOF
   
          unsigned char ack = rcvBuff[0];
          packNum = (int)ack;


          slideSize[packNum%WINDOW_SIZE] = n-1;
          memcpy(slideWindow[packNum%WINDOW_SIZE], rcvBuff+1,n-1);
          slideCount[packNum%WINDOW_SIZE] = packNum;


          //total ++;

          // check if the first pos in window
          if(packNum == windSd%256){
                // semd positive ack
                unsigned char PAck[2];
                PAck[0] = '+';
                PAck[1] = packNum; 
                mysendto(sockFd,PAck,2,0,(struct sockaddr *)&servStore,sizeof(servStore),3,1);
                //int mywindSd = windSd;
                //start the repeat timeout
                setitimer(ITIMER_REAL, &tout_val,0);
                //ualarm(TIMEOUT,0);
                int tmp = 0;
                //try to skip the existing packets in the sliding window when finding the start index of sliding 
                for(i=0;i<WINDOW_SIZE;i++){
                    //printf("%d(%d) ",slideCount[(windSd+i)%WINDOW_SIZE], (windSd+i)%256);
                    if(slideCount[(windSd+i)%WINDOW_SIZE] == (windSd+i)%256){
                        write(fp, slideWindow[(windSd+i)%WINDOW_SIZE], slideSize[(windSd+i)%WINDOW_SIZE]);
                        tmp++;
                        
                    }
                    else{break;}
                }
                windSd += tmp;                   
          }

        }
        else{
          printf("EOFEOF\n");
          close(fp);
          break;
        }
          lockWind = false; // unlock, now we can check
    
        }
        
        close(fp);  //close the file descripter   
     
}
