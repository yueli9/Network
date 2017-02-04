#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define CMD       "cal"

int main(int argc, char *argv[])
{
    int sockFd, portNum, n;
    struct sockaddr_in svaddr;
    struct hostent *server;
    
    char buf[256];
    bzero(buf,256);

    char Rbuf[256];
    bzero(Rbuf,256);

    char secKey[20];
    bzero(secKey,20);

    if (argc < 4) {
        printf("cmdclient hostname portnumber secretkey.\n");
        exit(0);
    }

    server = gethostbyname(argv[1]);
    if (server == NULL){           // get server name
        printf("ERROR no such hostname");
        exit(1);
    }

    portNum = atoi(argv[2]);                                // get port number  
    strcpy(secKey,argv[3]);                                 // get host key

    sockFd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockFd <0){       
        printf("ERROR openning socket");
        exit(1);
    }

    memset(&svaddr, 0, sizeof(svaddr));

    svaddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&svaddr.sin_addr.s_addr, server->h_length);
    
    svaddr.sin_port = htons(portNum);

    if (connect(sockFd,(struct sockaddr *) &svaddr,sizeof(svaddr)) < 0) {       
        printf("ERROR creating socket");
        exit(1);
    }

    sprintf(buf, "$%s$%s", secKey, CMD); 

    ssize_t wlen = write(sockFd,buf,strlen(buf));
    if (wlen != strlen(buf)) 
        perror("Error: write to socket ");

    memset(&Rbuf,0,sizeof(Rbuf));

    ssize_t rlen = read(sockFd,Rbuf,sizeof(Rbuf));
    if (rlen < 0) 
         perror("ERROR reading from socket");

    printf("%s\n",Rbuf);
    close(sockFd);
    return 0;
}
