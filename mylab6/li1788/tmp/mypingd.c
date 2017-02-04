/**
SERVER
**/
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/signal.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#define BUFSIZE 1000
#define SECRETKEY "ddmt7OOOaMLa"

int verify(char* msg, char* arg){
	if(strlen(msg)!=1000){
		return -1;
	}
	char* secretKey=strtok(msg, "$");
	if(strcmp(secretKey, arg)==0)
		return 1;
	return -1;
}
int check_format(char* key){
	int numeric;
	int charac;
	numeric=-1;
	charac=-1;
	if(strlen(key)<10 || strlen(key)>20){
		return -1;
	}
	int i;
	for(i=0; i<strlen(key); i++){
		if((key[i]>='a' && key[i]<='z') || (key[i]>='A' && key[i]<='Z')){
    		charac=1;
		}
    	else if(key[i] >= '0' && key[i] <= '9' ){
    		numeric=1;
    	}
	}
	return charac==1 && numeric==1 ? 1:-1;
}

int main(int argc, char *argv[])
{
	if(argc==3){
		int server_fd, counter;
		struct sockaddr_in s_address, c_address;
		socklen_t len;		
		char buff[BUFSIZE];
		char msg[5]="terve";
		counter=0;

		if(check_format(argv[2])==-1){
			perror("INVALID FORMAT OF KEY\n");
			exit(EXIT_FAILURE);
		}	

		if ((server_fd=socket(AF_INET, SOCK_DGRAM, 0))==-1){
			printf("socket created error\n");
			exit(EXIT_FAILURE);
		}

		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0){
    		perror("setsockopt(SO_REUSEADDR) failed");
    		exit(EXIT_FAILURE);
		}

		memset(&s_address, '\0', sizeof(s_address));
		
		
		s_address.sin_family=AF_INET;
		s_address.sin_addr.s_addr=htonl(INADDR_ANY);
		s_address.sin_port=htons(atoi(argv[1]));

		if (bind(server_fd, (struct sockaddr *)&s_address, sizeof(s_address))<0){
			perror("ERROR: BINDING ERROR");
			exit(EXIT_FAILURE);		
		}
		for(;;){
			printf("receving........\n");
			len=sizeof(c_address);
			if(recvfrom(server_fd, buff, 1000, 0, (struct sockaddr *)&c_address, &len)<0){
				perror("ERROR: RECV ERROR\n");
				exit(EXIT_FAILURE);
			}
			if(++counter==4){
				counter=0;
				printf("UNLUCKY GUY, IGNORE!\n");
				continue;
			}
			printf("VERIFYING............\n");
			if(verify(buff, argv[2])==1){
				printf("VERIFIED!!\n");	
				if(sendto(server_fd, msg, strlen(msg), 0, (struct sockaddr *)&c_address, sizeof(c_address))<0){
					perror("ERROR: SENDING ERROR\n");
					exit(EXIT_FAILURE);
				}
			}
			else{
				printf("WARNING: KEY IS NOT MATCHED!\n");
				continue;
			}		
		}

	}
	else{
		perror("INVALID INPUT\n");
		exit(EXIT_FAILURE);
	}
}
