#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/time.h>

//#define debug 1
//final variables can be tuned for performance
#define PACKET_SIZE 1000
#define WINDOW_SIZE 4
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define TIMEOUT 1000


char _window[WINDOW_SIZE][PACKET_SIZE+1];   //window declaration
int _window_size[WINDOW_SIZE];    //each packet size

//handle the zombie process by executing waitpid...

/*
void myhandler(int sig){
  wait3(0,0,NULL);
  while(waitpid(-1, NULL, WNOHANG) > 0);;
}

*/

//mysendto to substitute sendto with probability;
int send_counter = 0;
ssize_t mysendto(int sockFd, const void *buf, size_t len, int flags,
                      const struct sockaddr *dest_addr, socklen_t addrlen, int totalnum, int lossnum){
              int res;    
            send_counter ++;
      //printf("==%d==",send_counter);
            printf("Sendto %s\n", buf);
        float rate = (double)lossnum/totalnum;  
        float ran = ((double)rand() / (double)(RAND_MAX)); 
        //printf("%f \n",ran);   
        //tune ran to get different loss rate for packets       
            if(ran <= 0.9){
              res = sendto(sockFd, buf, len, flags, dest_addr, addrlen); 
            }else res = 0;
        return  res;        
}

//global variables
int count;    //record the current position read in file
struct sockaddr_in client_addr;   //child socket for child process
int child_socket; //child socket for child process
int prev_count = 0;   //
int _end = 0; //check if file read end
int first_flag = 0;   //for initial packets sending

/*
*only for initial packets send timeout, if all packets are lost for the initial sending, then the client will not
*know the ip and port of server's child process
*retry every TIMEOUT time to send the initial packets
*
*/
void timeout_handler(int sig){

  if(first_flag == 1)return;
  int i;
  //printf("Retry...\n");
  //window iteration
  if(count == prev_count){
    for(i=0;i<= MIN(WINDOW_SIZE-1,count);i++){
      //printf("Sending %d\n",_window[i][0]);
      mysendto(child_socket,_window[i],_window_size[i],0,(struct sockaddr *)&client_addr,sizeof(client_addr),200,1);            //count = 0;
    }
  }
  prev_count = count;
  struct itimerval tout_val;
  //restart the timer for repeat sending
  tout_val.it_interval.tv_sec = 0;
  tout_val.it_interval.tv_usec = 0;
  tout_val.it_value.tv_sec = 0; 
  tout_val.it_value.tv_usec = TIMEOUT;
  setitimer(ITIMER_REAL, &tout_val,0);
  
}


void main(int argc, char * argv[]){
  //argument check
  if(argc != 5){
    printf("ERROR turboserver portnumber secretkey configfile.dat lossnum\n");
    exit(1);
    }
  int status;


    if(strlen(argv[2])<10 || strlen(argv[2])>20){                                           // check secrete key
         printf("ERROR,security key length should be from 10 to 20\n");
         exit(1);
    }

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









    struct timeval start_time, end_time;
    //sig for timeout
    struct sigaction sact_t;
    sigemptyset( &sact_t.sa_mask );
    sact_t.sa_flags = SA_RESTART;
    sact_t.sa_handler = timeout_handler;
    sigaction(SIGALRM, &sact_t, NULL);
    
    /*
    //register the sigaction 
    struct sigaction sact;
    sigemptyset( &sact.sa_mask );
    sact.sa_flags = SA_RESTART;
    sact.sa_handler = myhandler;
    


    //use SIGCHLD to send signal when child process ends
    int error_z = sigaction(SIGCHLD, &sact, NULL);
    if ( error_z ) 
    {
    perror( "sigaction" );
    exit( -1 );
    }
  */
    
    //declare the socket and bind port number
    int sockFd;
    int i;
    struct sockaddr_in servAddr;
    socklen_t len;
    unsigned char buff[1024];
    char send_buff[1024];
    //init send_buff for payload...
    for(i=0;i<1024;i++)buff[i] = 0;
    for(i=0;i<1024;i++)send_buff[i] = 0;
    
    //server socket creation
    sockFd = socket(AF_INET, SOCK_DGRAM, 0);
    //clear the sockaddr
    
    int PortNum = atoi(argv[1]);
    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servAddr.sin_port = htons(PortNum);

    if((bind(sockFd,(struct sockaddr *)&servAddr,sizeof(servAddr)))<0 )
     { printf("ERROR binding to socket,\n");   //bind error
      exit(1);
      }



    char secKey[20];
    bzero(secKey,20);
    strcpy(secKey,argv[2]);                                             // get secretekey

    
    int prev_count = -1;
    int skip = 0;
    while(1){
      //int connfd;
      //accept block and wait for message coming in
      //connfd = accept(sockFd,(struct sockaddr *)&client_addr,&len);
      
      int n;
      //recvfrom the socket
      //n = read(sockFd, buff, 1024);
      len = sizeof(client_addr);
      //blocked, wait for new client request
      n = recvfrom(sockFd, buff, 1024, 0, (struct sockaddr *)&client_addr,&len);
      printf("Received file path: %s\n", buff);
      


      char csecKey[20];
      char *token;
      printf("buff %s\n ",buff);
      token = strtok(buff, "$");
      strcpy(csecKey,token);                         // get secrete key from buf

      if(strcmp(csecKey,secKey) != 0){               // check if secrete key match
        printf("secretkey does not match \n");
        continue;
      }

      token = strtok(NULL, "$");                    // get file name from buf
      printf("buf after: %s\n ",buff);  


      char filename[50];
      sprintf(filename, "%s%s", "./filedeposit/", token); 



      if(access(filename, F_OK | R_OK) != 0){   //check the file path
        printf("File does not exit\n");
        continue;
      }




        int  k = fork();
        //identify the child process with pid 0
        if(k == 0){

          float first_end_time = 0;  //get the time when file reach the end
          struct sockaddr_in self_addr;
          child_socket = socket(AF_INET, SOCK_DGRAM, 0);    //create  a new socket for file transfering
          bzero(&self_addr, sizeof(self_addr));
          self_addr.sin_family = AF_INET;
          self_addr.sin_addr.s_addr=htonl(INADDR_ANY);
            
          //random generate port number
          unsigned int tunnel_port = (rand()%65535)+1;
          //get a free port number
          self_addr.sin_port = htons(tunnel_port);
          //bind a random port
          while(bind(child_socket,(struct sockaddr *)&self_addr,sizeof(self_addr)) == -1){
        
            tunnel_port = (rand()%65535)+1;
            self_addr.sin_port = htons(tunnel_port);
        
          }

          int fp, rd;
          fp = open(filename, O_RDONLY);
          if(fp <= 1){
            char *bad = "Could not open file\n";
            //write(connfd,"INVALID REQUEST",15);
            write( 1, bad,  20  );
          }
          else{
            unsigned char c[PACKET_SIZE+1];
            //c[0] = 'A';
            gettimeofday(&start_time, NULL);  //get start time
            count = 0;
            int _counter = 0;
            while((rd=read(fp,c+1,PACKET_SIZE)) != 0 || _counter <= count){   //read the file
              
              //write(connfd,c,rd);   //send to the client
              c[0] = count;
              //write(1, c+1, rd);
              //printf(" :%d\n",count);
              if(rd!=0){
                memcpy(_window[count%WINDOW_SIZE], c, rd+1);
                _window_size[count%WINDOW_SIZE] = rd+1;
              }

              int cycle = 0;
              //waiting for client ACK
              while(count >= WINDOW_SIZE-1 || rd == 0){
                  int i;
                  //first time to send the packets
                  if(first_flag == 0){
                    for(i=0;i<MIN(WINDOW_SIZE,count);i++){
                      printf("Sending %d\n",_window[i][0]);
                      mysendto(child_socket,_window[i],_window_size[i],0,(struct sockaddr *)&client_addr,sizeof(client_addr),3,1);            //count = 0;
                    }
                    
                  }
                  //printf("Waiting for ack...\n");
                  //time out for first WINDOW_SIZE packets sending
                    /*
                    struct itimerval tout_val;
  
                    tout_val.it_interval.tv_sec = 0;
                    tout_val.it_interval.tv_usec = 0;
                    tout_val.it_value.tv_sec = 0; 
                    tout_val.it_value.tv_usec = TIMEOUT;
                    setitimer(ITIMER_REAL, &tout_val,0);
                    */
                  ualarm(TIMEOUT,0);
                    //need to read how many block more for file according to positive ACKs
                  if(skip > 0){skip --;break;}
                  //printf("stuck\n");
                  int rn;
                  //preparing for ending the child process
                  if(_end == 1){
                    float cur_time_taken = ((end_time.tv_sec * 1000.0 + end_time.tv_usec/1000.0) - (start_time.tv_sec * 1000.0 + start_time.tv_usec/1000.0));
                    
                    if(cur_time_taken - first_end_time > 1000.0){
                      printf("Time taken: %lf\n", cur_time_taken - 1000);
                      printf("End Child Process\n");
                      //ending the process by jump out of the outter loop
                      _counter = count + 1;
                      break;
                    }
                  }
                  //block for receiving ACKs 
                  else
                    {
                      bzero(buff, sizeof(buff));
                      rn = recvfrom(child_socket, buff, 1024, 0, (struct sockaddr *)&client_addr,&len);
                      printf("Received %s\n",buff );
                    }
                  //at least get some thing...
                  first_flag = 1;
                  

                  //case for negative ACKs
                  if(buff[0] == '-'){
                    
                    buff[0] = 0;
                    int miss = (int)buff[1];
                    printf("Miss number:%d %d\n",miss,count);
                    //resend the packet if packet is within range
                    if(miss < count){
                      miss = miss%WINDOW_SIZE;
                      mysendto(child_socket,_window[miss],_window_size[miss],0,(struct sockaddr *)&client_addr,sizeof(client_addr),3,1);
                    }
                  }
                                    
                  //printf("Received  %s\n", buff);
                  //case for positive ACKs
                  if(buff[0] == '+'){
                    buff[0] = 0;
                    int tmp = (int)buff[1];
                    
                    printf("got packet %d\n", tmp);
                    //check the if the positive ACK is consecutive
                    if(prev_count != tmp && 1){ 
                      if(tmp - prev_count >= 0){
                        _counter += tmp-prev_count ;
                        skip = tmp - prev_count;
                      }
                      else{
                        _counter += 256+tmp-prev_count;
                        skip = tmp - prev_count+256;
                      }
                      prev_count = tmp;
                    }
                    else{
                      prev_count = tmp;                      
                    }
                    
                  }
                  //for end of file detection
                  if(_counter == count && rd == 0 ){
                      char *end = "EOFEOF";
                      gettimeofday(&end_time, NULL);    //get end time
                      float time_taken = ((end_time.tv_sec * 1000.0 + end_time.tv_usec/1000.0) - (start_time.tv_sec * 1000.0 + start_time.tv_usec/1000.0));
                      //get the first time reach here
                      if(_end == 0){
                        first_end_time = time_taken;
                        printf("End of file, waiting for server timeout...\n");
                      }
                      _end = 1;
                      mysendto(child_socket,end,4,0,(struct sockaddr *)&client_addr,sizeof(client_addr),3,1); 
 
                  }
              } // while end
              //increment the index
              if(rd!=0){
                //cycle = count/256;
                count++;
              }
              
            }
            //printf("Bad\n");
          }
          //clean up
          close(rd);
          close(child_socket);
          //terminate the child process
          exit(1);
        }
        
          else if(k >0 ) {                                    //parent process  
        //close(newSockFd);
          printf("parent prosee \n");
          waitpid(k, &status,WNOHANG);                        //wait for child process status to change
          printf("child terminated");
        }
        else{         //fork failed exit
          printf("fork failed");  
          exit(1);
        }
            
      //loop back
      //close socket for child process
      //close(sockFd);
        
    }
    


}
