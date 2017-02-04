#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>


void main(int argc, char const *argv[])
{

	if (argc ! = 6)
	{
		printf("myfileclient hostname portnumber secretkey filename configfile.dat \n");
		exit(1);

	}


	int portNumber = atoi(argv[2]);
	struct sockaddr_in severAddr, clntAddr;
	bzero(&severAddr,sizeof(severAddr));
	bzero(&clntAddr,sizeof(clntAddr));

	// setup server address
	severAddr.sin_family = AF_INET;
	severAddr.sin_addr.s_addr=inet_addr(argv[1]);
	severAddr.sin_port = htons(portNumber);

	int serverSize = sizeof(severAddr);
	int	sockFd = socket(AF_INET, SOCK_DGRAM, 0);

	char sbuf[1024];
	bzero(sbuf,1024);

	// send seckey and filename to server;
	if (sizeof(argv[4])>16)
	{
		printf("ERROR,filename could not exced 16 character\n");
	}

	int i =0;
	for (i =0;i<sizeof(argv[4]);i++)
	{
		if (argv[4][i] == '/')
		{
			printf("filename should not contain '/'\n");
			exit(1);
		}
	}

	sprintf(sbuf,"$%s$%s",argv[3],argv[4]);

	if (sendto(sockFd,sbuf,1024,0,(struct sockaddr_in *)&severAddr,sizeof(severAddr))<0)
	{
		printf("ERROR send to server\n");	
		exit(1);
	}
	// open or create the file
	int fp = open(argv[4], O_WRONLY|O_CREAT|O_TRUNC, 0644);


	while(1){

		char recvBuff[4096];
		bzero(recvBuff,4096);
		int n=recvfrom(sockFd,recvBuff,PACKET_SIZE+1,0,(struct sockaddr*)&severAddr, &serverSize);

		if (n<=0)
		{
			break;
		}
		


	}




















	return 0;
}
