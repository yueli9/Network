#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <fcntl.h>

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
    
    char fileName[20];
    bzero(fileName,20);

    char bufBlockSize[10];
    int blocksize=0;


    if (argc < 6) {
        printf("fileclient hostname portnumber secretkey filename configfile.dat \n");  // check arguments number
        exit(0);
    }

    server = gethostbyname(argv[1]);
    if (server == NULL){                                    // get server name
        printf("ERROR no such hostname");
        exit(1);
    }

    portNum = atoi(argv[2]);                                // get port number  
    strcpy(secKey,argv[3]);                                 // get host key

    if(strlen(argv[3])<10 || strlen(argv[3])>20){
        printf("ERROR,security key length should be from 10 to 20\n");
        exit(1);    
    }

    strcpy(fileName,argv[4]);                               // get fileName
    int fi =0;
    for(fi = 0;fi<strlen(fileName);fi++){
        if(fileName[fi] == '/'){                            // make sure not '/' exist
            printf("could not include '/'' in fileName");
        }
    }

    if( access( fileName, F_OK ) != -1 ) {                  // check if file already exist
            printf("File already exist .\n");
            exit(1);
    } 
                                                             // if file doesn't exist

    FILE *fp = fopen(fileName, "ab");                       // create the filename
    
    int bsfd = open(argv[5],O_RDONLY);                       // buffer size

    int nbs = read(bsfd,bufBlockSize,sizeof(bufBlockSize)-1);
    if(nbs >=16)
    {

        printf("filename should not exced 16 characters.\n");
        exit(1);
    }

    bufBlockSize[nbs] ='\0';
    blocksize = atoi(bufBlockSize);
    printf("blocksize is %d \n",blocksize);


    sockFd = socket(AF_INET, SOCK_STREAM, 0);               // open socket

    if (sockFd <0){       
        printf("ERROR openning socket");
        exit(1);
    }

    memset(&svaddr, 0, sizeof(svaddr));

    svaddr.sin_family = AF_INET;                            // get server address
    bcopy((char *)server->h_addr, (char *)&svaddr.sin_addr.s_addr, server->h_length);
    
    svaddr.sin_port = htons(portNum);
                                                            // create socket
    if (connect(sockFd,(struct sockaddr *) &svaddr,sizeof(svaddr)) < 0) {       
        printf("ERROR creating socket");
        exit(1);
    }

    sprintf(buf, "$%s$%s", secKey, fileName);               // create the message to send

    ssize_t wlen = write(sockFd,buf,strlen(buf));            // send msg to server
    if (wlen != strlen(buf)) 
        perror("Error: write to socket ");

    shutdown(sockFd, SHUT_WR);                                 // shut down the write end

    memset(&Rbuf,0,sizeof(Rbuf));

    char rdbuff[blocksize];
    memset(rdbuff, '0', sizeof(rdbuff));
    int byterd = 0;


    struct timeval start, end;

    gettimeofday(&start, NULL);

    int filesz  =0;
    while((byterd = read(sockFd, rdbuff, blocksize)) > 0)       // keep reading from server
    {
        filesz += byterd;                                       // record file size
        printf("Bytes received from server :%d\n",byterd);      
        fwrite(rdbuff, 1,byterd,fp);
    }

    if(byterd < 0)
    {
        printf("\n ERROR reading from server \n");
    }

    gettimeofday(&end, NULL);           


    unsigned long totalt =  (end.tv_sec* 1000000  + end.tv_usec )      // calculate time spend
          - (start.tv_sec *1000000  + start.tv_usec);

    printf("\n completion time %ld usec, bytes transferred %d, reliable throughput %d mbps \n ",totalt,filesz,filesz/totalt);


/*
    ssize_t rlen = read(sockFd,Rbuf,sizeof(Rbuf));          // receive msg from server
    if (rlen < 0) 
         perror("ERROR reading from socket");

    printf("%s\n",Rbuf);

*/
    close(sockFd);
    return 0;
}
