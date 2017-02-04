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
#include <ctype.h>
#define BUF_SIZE 4096
#define PATH "./filedeposit/"

int get_dat_stat(char* file_name){
	printf("Getting the Blocksize\n");
	int f;
	char blockSize[BUF_SIZE];
	if((f=open(file_name, O_RDONLY, 0644)) < 0){
		perror("ERROR: CAN NOT OPEN DAT FILE");
		return -1;
	}
	read(f, &blockSize, BUF_SIZE*sizeof(char));
	printf("The Block Size is : %d\n", atoi(blockSize));
	return atoi(blockSize);
}
int verify(char* cli_key, const char* ser_key){
	if(strcmp(cli_key, ser_key)==0){
		return 1;
	}
	return -1;
}
int send_file(int sock, int block_size, char* file_name){
	int sent_count, read_bytes, sent_bytes, sent_file_size;
	char send_buf[block_size];
	char *err_msg = "File not found";
	char fullpath[BUF_SIZE];
	int fd;
	sent_count=0;
	sent_file_size=0;
	int n;
	n = sprintf(fullpath,"%s%s",PATH, file_name);
	if(n<0) error("ERROR in sprintf");
	if((fd=open(fullpath, O_RDONLY))<0){
		perror(file_name);
		exit(EXIT_FAILURE);
	}else{
		printf("Sending...............\n");
		while((read_bytes=read(fd, &send_buf, block_size))>0){
			if( (sent_bytes=write(sock, (void *)send_buf, block_size))< read_bytes){
				perror("Send ERROR");
				exit(EXIT_FAILURE);
			}
			sent_count++;
			sent_file_size+=sent_bytes;
		}
		close(fd);
	}
	printf("Down with this client, Sent %d bytes in %d send\n", sent_file_size, sent_count);
	return sent_count;
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
	/* code */
	if(argc==4){
	int server_fd;
	int client_fd;
	int s_len, c_len, b_size;
	struct sockaddr_in s_address;
	struct sockaddr_in c_address;
	int port;

	if(check_format(argv[2])==-1){
		perror("INVALID FORMAT OF KEY\n");
		exit(EXIT_FAILURE);
	}
	/**CREATE THE SOCKET**/
	if ((server_fd=socket(AF_INET, SOCK_STREAM, 0))<0){
		perror("ERROR: CAN NOT CREATING SOCKET\n");
		exit(EXIT_FAILURE);
	}
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0){
    	error("setsockopt(SO_REUSEADDR) failed");
    	exit(EXIT_FAILURE);
	}

	memset(&s_address, '\0', sizeof(s_address));
	/**SET THE INFO FOR ADDR**/
	s_address.sin_family=AF_INET;
	s_address.sin_addr.s_addr=htonl(INADDR_ANY);
	port=atoi(argv[1]);
	s_address.sin_port=htons(atoi(argv[1]));
	s_len=sizeof(s_address);

	/** BIND THE VALUE**/
	printf("BINDING \n");
	if(bind(server_fd, (struct sockaddr*)&s_address, s_len)<0){
		perror("ERROR: BINDING ERROR\n");
		exit(EXIT_FAILURE);
	}
	printf("LISTENING\n");
	/** Listen **/
	if(listen(server_fd, 5)<0){
		perror("ERROR: LISTENING ERROR\n");
		exit(EXIT_FAILURE);
	}
	b_size=get_dat_stat(argv[3]);
	while(1){
		char req[b_size];
		bzero(&req, sizeof(req));
		c_len=sizeof(c_address);   
 		printf("ACCEPTING\n");
		client_fd=accept(server_fd, (struct sockaddr *)&c_address, &c_len);	
		read(client_fd, &req, BUF_SIZE*sizeof(char));
		char* key= strtok(req, "$");
		/** Check the Key**/
		if(verify(key, argv[2])!=1){
			printf("%s %s\n", key, argv[2]);
			perror("Authication error\n");
			exit(EXIT_FAILURE);
		}	
		char* file_name=strtok(NULL, "$");
 		if(fork()==0){	
 			if (close(server_fd) < 0){
				perror("Client--close listener\n");
				exit(EXIT_FAILURE);
 			}
 			/** Send the file from the server **/
 			send_file(client_fd, b_size, file_name);
 			exit(0);
 		}else{
 			close(client_fd);
 		}
	}
	}
	else{
		perror("Not Enough Arguments\n");
	}
}
