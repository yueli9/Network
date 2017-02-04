// simple shell example using fork() and execlp()

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{

int status;
int len;
char *token;
char *Cmd;

int fd;

char *msg[30];

socklen_t ctN;


if (argc < 4) {                                                                     // check arguement number
     printf("ERROR,lacking port number or security key or configfile.dat\n");
     exit(1);
 }

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

        printf("filename should not exced 16 characters.\n");                   // check filename size
        exit(1);
    }

    bufBlockSize[nbs] ='\0';
    blocksize = atoi(bufBlockSize);                                             // get blocksize
    printf("blocksize is %d \n",blocksize);





int sockFd;
int portNum;
int pid;
char secKey[20];
bzero(secKey,20);

struct sockaddr_in svaddr,ctaddr;

portNum = atoi(argv[1]);                                            // get port number
strcpy(secKey,argv[2]);                                             // get secretekey

memset(&svaddr, 0, sizeof(svaddr));
svaddr.sin_family = AF_INET;
//svaddr.sin_addr.s_addr = htonl(ipv4address);
svaddr.sin_addr.s_addr = INADDR_ANY;
svaddr.sin_port = htons(portNum);

sockFd = socket(AF_INET,SOCK_STREAM,0);                                 // set up socket

if(sockFd <0){
	printf("ERROR cereating socket\n");
     exit(1);
}

if(bind(sockFd,(struct sockaddr *) &svaddr, sizeof(svaddr)) <0){            // error check
	printf("ERROR binding socket\n");
	 exit(1);
}


listen(sockFd,5);                                                           // wait for msg from client

ctN = sizeof(ctaddr);
while(1) {                                                                   //keep server alive
	int newSockFd = accept(sockFd,(struct sockaddr *) &ctaddr,&ctN);           // receive msg from client
	if(newSockFd <0){
		printf("ERROR accepting socket\n");
		exit(1);
	}

   char buf[256];
   bzero(buf,256);
   int n = read(newSockFd,buf,sizeof(buf)-1);       // read msg from socket
   buf[n] = '\0';

   	if (n < 0) {
		printf("ERROR reading socket\n");
		exit(1);
	}

  	if(n == 0)
		continue;
	
	char csecKey[20];
	
	printf("buf %s\n ",buf);
    token = strtok(buf, "$");
	strcpy(csecKey,token);  		         	         // get secrete key from buf

	if(strcmp(csecKey,secKey) != 0){               // check if secrete key match
		printf("secretkey does not match \n");
		continue;
	}

	token = strtok(NULL, "$");			              // get message from buf
	printf("buf after: %s\n ",buf);  

    int l=0;
	while (token != NULL) {                       // extract filename from msg
	printf("filename is :%s",token);
	msg[l] = token;
	token = strtok(NULL, "$");
	l++;
	}
	msg[l] =NULL;


  	int k = fork();
  	if (k==0) {							             // child process

  	
  		char filename[50];
    	sprintf(filename, "%s%s", "./filedeposit/", msg[0]); 

    	printf("filedir is %s\n",filename);
        FILE *fp = fopen(filename,"rb");
        if(fp==NULL)
        {
            printf("\n ERROR open file!! \n");
            exit(1);
            return 1;
        }

        while (1)					                                // start reading from file 
        {
            char rbuf[blocksize];
            bzero(rbuf,blocksize);

            int nr = fread(rbuf,1,blocksize,fp);          // read from file
            printf("Read %d Bytes success.%d \n", nr);

            if(nr > 0)							                   // start sending bytes
            {
                write(newSockFd, rbuf, nr);
                bzero(rbuf,blocksize);

            }

            if (nr < blocksize && nr >=0)				       // check EOF
            {
                if (feof(fp))
                    printf("End of file\n");
             
                break;
            }

            if (nr < 0)				                       // check EOF
			{
				if (ferror(fp))
					printf("Error reading file \n");
			    break;
			}
        }
		close(newSockFd);		                             	// close sock
  	}	

	else if(k >0 ) {			                              //parent process	
	close(newSockFd);
	printf("parent prosee \n");
	waitpid(k, &status,WNOHANG);		                   	//wait for child process status to change
  	printf("child terminated");
	}
	else{					//fork failed exit
	printf("fork failed");	
	exit(1);
	}  
  }

close(sockFd);
return 0;
}
