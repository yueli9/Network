	/**Client**/
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

volatile sig_atomic_t count=0;
void handler(int sig){
	count++;
	if(count >= 2550000/50000){
		printf("No response from the server\n");
		exit(1);
	}

}
int verify(char* msg){
	char *tmp="terve";
	if(strcmp(msg, tmp)==0){
		return 1;
	}
	return -1;
}
void initializeReq(char buff[], char* secretKey){
	int i;
	strcpy(buff, "$");
	strcat(buff, secretKey);
	strcat(buff, "$");
	for(i=strlen(secretKey)+2; i<1000; i++){
		if(i<500){
			buff[i]='a';
		}	
		else{
			buff[i]='0';
		}
	}
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
	if(argc==4){
		printf("The ip %s and port number is %s and key is %s\n", 
			argv[1], argv[2], argv[3]);
		
		struct timeval send_t, recv_t;
		struct sockaddr_in s_address, verify_addr, myaddr;
		int fd;
		socklen_t source_size=sizeof(verify_addr);
		char buff[1000];

		/** Create the requset**/
		initializeReq(buff, argv[3]);

		if(check_format(argv[3])==-1){
			perror("INVALID FORMAT OF KEY\n");
			exit(EXIT_FAILURE);
		}	
		/** Create the socket **/
		if ((fd=socket(AF_INET, SOCK_DGRAM, 0))==-1){
			printf("socket created error\n");
			exit(EXIT_FAILURE);
		}

		if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0){
    		perror("setsockopt(SO_REUSEADDR) failed");
    		exit(EXIT_FAILURE);
		}
		
		memset((char *)&myaddr, 0, sizeof(myaddr));
		myaddr.sin_family = AF_INET;
		myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		myaddr.sin_port = htons(0);

		if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
			perror("bind failed");
			return 0;
		}       
		/** BIND THE SOCKET**/
		memset(&verify_addr, '\0', sizeof(verify_addr));
		memset(&s_address, '\0', sizeof(s_address));

		s_address.sin_family=AF_INET;
		s_address.sin_addr.s_addr=inet_addr(argv[1]);
		s_address.sin_port=htons(atoi(argv[2]));

		/** Get the send time **/
		if (gettimeofday(&send_t, NULL)<0){
			perror("ERROR: CAN NOT GET TIME OF DAY");
			exit(EXIT_FAILURE);
		}
		printf("Sending the request %s\n", buff);
		int n;
		n=sendto(fd, buff, 1000, 0, (struct sockaddr *)&s_address, sizeof(s_address));
		while(1){
			char resp[5];
			int res;
			bzero(&resp, sizeof(resp));
			printf("Receving............\n");
			signal(SIGALRM, handler);
			ualarm(50000, 50000);
			recvfrom(fd, resp, 5, 0, (struct sockaddr *)&verify_addr, &source_size);
			res = verify(resp);
			if(res==1){
				printf("VERIFIED!\n");
				gettimeofday(&recv_t, NULL);
		  		printf("%lf\n", ((recv_t.tv_sec * 1000.0 + recv_t.tv_usec/1000.0) - 
		  			(send_t.tv_sec * 1000.0 + send_t.tv_usec/1000.0)));
		  		printf("ip address: %s port: %u\n",inet_ntoa(*(struct in_addr *)&verify_addr.sin_addr.s_addr), ntohs(verify_addr.sin_port));
		  	}
		  	else{
		  		printf("ERROR: FAILED VERIFICATION\n");
		  		exit(EXIT_FAILURE);
		  	}
		  	break;
		}
		
	}
	else{
		printf("WRONG INPUT\n");
		exit(EXIT_FAILURE);
	}
}