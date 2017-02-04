#include <stdio.h>
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
#include <time.h>

int main(){
		int payloadSize =250;
    	int fd = open("pp.au", O_RDONLY);
	      char readBuffer[payloadSize];
        	bzero(readBuffer,payloadSize);
	    	 int audioFd=open("/dev/audio", O_WRONLY, S_IRUSR | S_IWUSR);
		 if(audioFd <0)
		 	printf("Failed open");
        	while(1){
			//usleep(1000);
	    	int rn = read(fd, readBuffer, payloadSize);
    		write(audioFd,readBuffer,strlen(readBuffer));
		}

}
