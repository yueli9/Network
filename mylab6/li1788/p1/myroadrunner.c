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

//#define debug 1
//SLIDING WINDOW size and Packet size
#define WINDOW_SIZE 4
#define PACKET_SIZE 1000
//#define TIMEOUT 900000

//timeout value for not receiving any SERVER message
/*this value need be tuned for different network situation, if network loses a lot of packets set this be small 
*and opposite otherwise
*e.g. 200 for non loss
* 	  50 for 50% loss
*/
#define TIMEOUT 100
//define window and some secondary window for size storage
char _window[WINDOW_SIZE][PACKET_SIZE+1];
int _window_size[WINDOW_SIZE];
int _window_checker[WINDOW_SIZE];

//the window starting index
int shift = 0;
int sockfd;

//define the socket as global varaiable
struct sockaddr_in sender;
struct itimerval timer;

//mutex lock for receive
int finish = 0;
int print_max = 1;
int send_counter = 0;
//mysend based on probability
ssize_t mysendto(int sockfd, const void *buf, size_t len, int flags,
                      const struct sockaddr *dest_addr, socklen_t addrlen, int totalnum, int lossnum){
              int res;    
	          send_counter ++;
			//printf("==%d==",send_counter);
			  float rate = (double)lossnum/totalnum;	
			  float ran = ((double)rand() / (double)(RAND_MAX)); 
			  //printf("%f ",ran);          
	          if(ran <= 0.9){
	          	res = sendto(sockfd, buf, len, flags, dest_addr, addrlen); 
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
	  		
			//printf("%d;;;;",finish);
			//mutex lock check, cannot do sig when is running during moving sliding window
			if(finish == 0){setitimer(ITIMER_REAL, &tout_val,0);return;}
			int i;
			 // printf("---------------------\n");
			unsigned char _ack[2];
			_ack[0] = '+';
			_ack[1] = shift%256-1; 
			//resend the lost postive ACK
			mysendto(sockfd,_ack,2,0,(struct sockaddr *)&sender,sizeof(sender),3,1);
			//printf("I think you are missing %d\n",shift%256-1);
			//if(shift == 4)exit(0);
			
			
			//Check which part of the window is not fill and send the negative ACK
			for(i=0;i<WINDOW_SIZE;i++){
				//printf("%d ",_window_checker[(shift+i)%WINDOW_SIZE]);
				
				//only when index match the packet number stored
				if(_window_checker[i] == -1 || _window_checker[(shift+i)%WINDOW_SIZE] != (shift+i)%256)				{
					//#ifdef debug
						printf("missing:%d\n",shift+i);
					//#endif
					unsigned char _ack[2];
					_ack[0] = '-';
					_ack[1] = shift+i;
					//send the packet
					mysendto(sockfd,_ack,2,0,(struct sockaddr *)&sender,sizeof(sender),3,1);
					//sendto(sockfd,_ack,2,0,(struct sockaddr *)&sender,sizeof(sender));
				}
				//printf("%d ",_window_checker[(shift+i)%WINDOW_SIZE]);
				//if(_window_checker[(shift+i)%WINDOW_SIZE] == shift+i){shift+=1;printf("Already got!\n");}
			}
			//printf("\n");
			
			//repeat timeout
	  		setitimer(ITIMER_REAL, &tout_val,0);

}

void main(int argc, char * argv[]){
	//only there are 2 arguments, the program will execute the following
	if(argc == 5){
		struct timeval send_time, recv_time;
		struct sigaction sact;
		sigemptyset( &sact.sa_mask );
		sact.sa_flags = SA_RESTART;
		sact.sa_handler = myhandler;
		sigaction(SIGALRM, &sact, NULL);
		//define the socket and ip address
		printf("ip: %s port: %s\n", argv[1], argv[2]);
		
		int i,n;
		//init the window buffer to store each packet size
		for(i=0;i<WINDOW_SIZE;i++)_window_checker[i] = -1;		
		struct sockaddr_in server_addr, client_addr;
		
		socklen_t sendsize = sizeof(sender);
		bzero(&sender, sizeof(sender));		//clear
		//struct scokaddr_in _sin;
		socklen_t len;
		char buff[1024];
		char recv_buff[1024];
		//define socket
		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		
		//clear the socketaddr
		bzero(&server_addr, sizeof(server_addr));
		//init ip address and port
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr=inet_addr(argv[1]);
		server_addr.sin_port = htons(atoi(argv[2]));
		for(i=0;i<strlen(argv[3]);i++)buff[i] = argv[3][i];	//init payload
		buff[i]=0;
		for(i=0;i<1024;i++)recv_buff[i] = 0;		//init payload
		
		printf("file path: %s\n",buff);
		printf("file save as: %s\n",argv[4]);
		
		//get time of the day
		gettimeofday(&send_time, NULL);
		//send a message to server
		//write(sockfd,buff,1024);
		
		//send a file path to server, if lost we can always rerun the app
		sendto(sockfd,buff,1024,0,(struct sockaddr *)&server_addr,sizeof(server_addr));
		
		unsigned char c_buf[4096];
		char bad_req[16];
		bad_req[15] = 0;
		int fp;
		fp = open(argv[4], O_WRONLY|O_CREAT|O_TRUNC, 0644);	//open file to write
		
		int count = 0;
		int total = 0;
		int prev = -1;
		
		//timeout sig init
		struct itimerval tout_val;
  
  		tout_val.it_interval.tv_sec = 0;
  		tout_val.it_interval.tv_usec = 0;
  		tout_val.it_value.tv_sec = 0; 
  		tout_val.it_value.tv_usec = TIMEOUT;
  		
  		//listen for new coming request
		while ((n=recvfrom(sockfd,c_buf,PACKET_SIZE+1,0,(struct sockaddr*)&sender, &sendsize)) > 0 )	//read the socket
		{
		  //printf("%s\n",c_buf);
		  //mutex lock
		  finish = 0;
		  if(strcmp(c_buf, "END") == 0){		//the end of message
			  printf("END\n");
			  close(fp);
			  break;
		  }
		  if(strcmp(c_buf, "INVALID REQUEST") == 0){		//bad request
			  printf("INVALID REQUEST\n");
			  close(fp);
			  remove (argv[4]);		//delete the file if is a invalid request
		  }
		  else{
			  //write(1,c_buf+1,n-1);
			  unsigned char ack = c_buf[0];
			  //copy the incoming message to the window accoring to its packet number
			  count = (int)ack;
			  _window_size[count%WINDOW_SIZE] = n-1;
			  memcpy(_window[count%WINDOW_SIZE], c_buf+1,n-1);
			  _window_checker[count%WINDOW_SIZE] = count;
			  //printf("Received %d\n",count);
			  total ++;
			  #ifdef debug
			 	printf("-------  %d %d %d %d--------\n",count,shift,n-1,total);
			  #endif
			  
			  if(shift-1 >= print_max){
			  		print_max *= 2;
				  printf("Received %d packets\n", shift-1);
			  }
			  //if(count < shift)shift = count + shif;
			  
			  //move the window if the first index of the window is filled...
			  if(count == shift%256){
			  		//construct a postive ACK
					unsigned char _ack[2];
					_ack[0] = '+';
					_ack[1] = count; 
					mysendto(sockfd,_ack,2,0,(struct sockaddr *)&sender,sizeof(sender),3,1);
					#ifdef debug
						printf("I got %d\n",count);
					#endif
					int myshift = shift;
					//start the repeat timeout
					setitimer(ITIMER_REAL, &tout_val,0);
					int tmp = 0;
					//try to skip the existing packets in the sliding window when finding the start index of sliding 
					for(i=0;i<WINDOW_SIZE;i++){
						//printf("%d(%d) ",_window_checker[(shift+i)%WINDOW_SIZE], (shift+i)%256);
						if(_window_checker[(shift+i)%WINDOW_SIZE] == (shift+i)%256){
							write(fp, _window[(myshift+i)%WINDOW_SIZE], _window_size[(myshift+i)%WINDOW_SIZE]);
							tmp++;
							
						}
						else{break;}
			  		}
					shift += tmp;					
			  }
		  }
		  //mutex unlock
		  finish = 1;
		  
		 
		}
		
		close(fp);	//close the file descripter
		
	}
	else{
		write(1,"Wrong input!\n",13);		//wrong input argument
	}
}
