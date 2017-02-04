// audiostreamd.c
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

void myplot();
void SIGIOHandler(int signalType);      //  SIGIO handler
void handle_alarm(int sig);             //  SIGIO handler
struct sockaddr_in udpClientAddr;
socklen_t udpClientln = sizeof(udpClientAddr);
static int rn =1 ; //record reading from file status

static int udpSockFd ;
static int tau;							// tau updated by sigpoll

void updateTau(int Qt, int Qstar, int gamma, int method);
int mode;
int fd;
int payloadSize;
static int i;
int lambdaArr[500000];
int timeArr[500000];
static int t =0;

static int alarmCounter = 0;
int audiobuf;
int main(int argc, char *argv[])
{

int status;

socklen_t ctN;


if (argc < 8) {
    printf("ERROR,audiostreamd tcp-port udp-port payload-size packet-spacing mode logfile-s audiobuf\n");
    printf("./audiostreamd 57666 58933 250 90 1 logs 30\n");
	exit(1);
 }


int tcpPort = atoi(argv[1]);
int udpPort = atoi(argv[2])-1;
 payloadSize = atoi(argv[3]);
int packetSpacing = atoi(argv[4])*1000;  // usec = 1000 * msec
mode = atoi(argv[5]);
char logfileS[20];
bzero(logfileS,20);
strcpy(logfileS,argv[6]);

audiobuf = atoi(argv[7]);


int sockFd;
struct sockaddr_in svaddr,ctaddr;
memset(&svaddr, 0, sizeof(svaddr));
memset(&svaddr, 0, sizeof(ctaddr));
ctN = sizeof(ctaddr);


svaddr.sin_family = AF_INET;
svaddr.sin_addr.s_addr = INADDR_ANY;
svaddr.sin_port = htons(tcpPort);

sockFd = socket(AF_INET,SOCK_STREAM,0);

if(sockFd <0){
	printf("ERROR cereating socket\n");
     exit(1);
}

if(bind(sockFd,(struct sockaddr *) &svaddr, sizeof(svaddr)) <0){
	printf("ERROR binding socket\n");
	 exit(1);
}

listen(sockFd,5);

while(1) {

	int newSockFd = accept(sockFd,(struct sockaddr *) &ctaddr,&ctN);
	if(newSockFd <0){
		printf("ERROR accepting socket\n");
		exit(1);
	}


    char buf[256];
    bzero(buf,256);
    int n = read(newSockFd,buf,sizeof(buf)-1);
    buf[n] = '\0';

   	if (n < 0) {
		printf("ERROR reading socket\n");
		exit(1);
	}

  	if(n == 0)
		continue;

	printf("Rceived:%s\n",buf );
	char *pch;
	int rcvUDPport;

	pch = strtok (buf," ");
	rcvUDPport = atoi(pch);
    pch = strtok (NULL," ");  					// pch is filename now


	char fb[48];
	bzero(fb,48);
	int m;
	if (access(pch,F_OK) != -1)
	{											// File exist
		udpPort++;
		sprintf(fb,"OK %d",udpPort);
		m = write(newSockFd, fb, strlen(fb));

  		int k = fork();
	  	if (k==0) {								// child process

	  	tau  = 0;

		struct sockaddr_in udpServerAddr;
		memset(&udpServerAddr, 0, sizeof(udpServerAddr));
		memset(&udpClientAddr, 0, sizeof(udpClientAddr));
		char rcvBuffer[1024];
		bzero(rcvBuffer,1024);


		udpServerAddr.sin_family = AF_INET;
		udpServerAddr.sin_addr.s_addr = INADDR_ANY;
		udpServerAddr.sin_port = htons(udpPort);

		udpSockFd = socket(AF_INET,SOCK_DGRAM,0);
		if(udpSockFd <0){
			printf("ERROR cereating udpSockFd\n");
     		exit(1);
		}
		if(bind(udpSockFd,(struct sockaddr *) &udpServerAddr, sizeof(udpServerAddr)) <0){
		printf("ERROR binding udpSockFd\n");
	 	exit(1);
		}


   		if (recvfrom(udpSockFd, rcvBuffer, 1024, 0, (struct sockaddr_in *) &udpClientAddr, &udpClientln) < 0){
        	printf("Failed receive from client\n");
        	exit(1);
        	}

        //printf("received:%s\n",rcvBuffer );
    	//tau = atoi(rcvBuffer);
		tau = packetSpacing;						// initial packet spacing

		struct sigaction handler;        				//Signal handling action definition 

    	handler.sa_handler = SIGIOHandler;
    	if (sigfillset(&handler.sa_mask) < 0) 
        	{printf("sigfillset() failed");
            exit(1);}

    	handler.sa_flags = 0;           				// no flags

    	if (sigaction(SIGIO, &handler, 0) < 0)
        	{printf("sigaction() failed for SIGIO");
        	exit(1);}

    	if (fcntl(udpSockFd, F_SETOWN, getpid()) < 0)
        	{printf("Unable to set process owner to us");
        	exit(1);}

    	if (fcntl(udpSockFd, F_SETFL, O_NONBLOCK | FASYNC) < 0)
        	{printf("Unable to put client sock into non-blocking/async mode");
            	exit(1);}


        //char readBuffer[payloadSize];
        //bzero(readBuffer,payloadSize);

        printf("%s\n",pch );
    	fd = open(pch, O_RDONLY);
    	i = 0;

    	signal( SIGALRM, handle_alarm );                // Install handler for ualarm,

    	//tau = 30000;
    	printf("Tau is %d \n",tau );
		//ualarm(tau,tau);
		ualarm(tau,10);



    	while(1){
    		if (rn <=0)
    		{
    			break;
    		}

/*
    		int rn = read(fd, readBuffer, payloadSize);
    		if (rn<=0)
    			break;

    		char sendbackBuffer[payloadSize+4];
    		bzero(sendbackBuffer,payloadSize+4);
    		sprintf(sendbackBuffer,"%04d%s",i,readBuffer);

    		printf("%s\n",sendbackBuffer );
			if(sendto(udpSockFd,sendbackBuffer,strlen(sendbackBuffer),0,(struct sockaddr *)&udpClientAddr,udpClientln) <0){
			printf("Failed to send back\n");	}

			i++;
			//printf("Tau is %d\n",tau );
			//sleep(tau/1000000);
			//usleep(tau%1000000);
			//tau = 1000000;
			//usleep(tau);

			int r = sleep(10000);   // why sleep does not work?

			printf("time spend %d\n",r );

			*/

    	}



		 myplot();

    	close(udpSockFd);
    	close(newSockFd);

    	printf("Socked closed !\n");

    	        FILE *fp;

        fp = fopen(logfileS, "w+");
        fprintf(fp, "This is testing for fprintf...\n");
        
        int l = 0;
        for(l =0 ;l<t;l++)
        {
            char bbuf[50] = {0}; 

            sprintf(bbuf, "%d %d\n", timeArr[l],lambdaArr[l]);

            fputs(bbuf, fp);
        }
        fclose(fp);

            exit(1);
        

	  	}	

		else if(k >0 ) {						//parent process	
		close(newSockFd);
		close(udpSockFd);

		printf("parent prcosee \n");
		waitpid(k, &status,WNOHANG);			//wait for child process status to change
	  	printf("child terminated");
		}
		else{									//fork failed exit
		printf("fork failed");	
		exit(1);
		}  

		
	}
	else{	// File not exist
		strcpy(fb,"KO");
		m = write(newSockFd, fb, strlen(fb));

	}


  }

close(sockFd);
return 0;
}



void SIGIOHandler(int signalType)
{


    struct sockaddr_in sndAddr;     		/* Address of sender */
    int sndAdLn = sizeof(sndAddr);          
    int recSz =0;                  
    char rcvBuf[1024]; 
    //printf("SIGIO action \n");      
    bzero(rcvBuf,1024);
    if ((recSz = recvfrom(udpSockFd, rcvBuf, 1024, 0, (struct sockaddr *) &sndAddr, &sndAdLn)) < 0)
        {
        	printf("Failed receive from client\n");
        	exit(1);
        }

    printf("received :%s\n",rcvBuf );



   	lambdaArr[t] = 1000000/tau;
    timeArr[t] = time(NULL);
    t++;



    char *pch;
    int rcvUDPport;


    if (rcvBuf[0] == 'Q')
    {
    	/* code */
    
    pch = strtok (rcvBuf," ");
    pch = strtok (NULL," ");                    // pch is portnumber
    
    int Qt = atoi(pch);
    pch = strtok (NULL," ");                    // pch is portnumber
    int Qstar = atoi(pch);
    pch = strtok (NULL," ");                    // pch is portnumber
    int gamma = atoi(pch);

    printf("Received Qt %d Q* %d gamma %d\n",Qt,Qstar,gamma);
    //updateTau( Qt,  Qstar,  gamma,  mode);

	}

}


void updateTau(int Qt, int Qstar, int gamma, int method){

	//int newtau =0;
	if (method == 1)
	{
		int a = 20;
		if (Qt >Qstar)
		{
			/* code */
			tau = tau + a;
		}
		if (Qt <Qstar)
		{
			/* code */
			tau = tau - a;
		}
	}


		if (method == 2)
	{
		int b = 10;
		int delta = 2; // delta >1
		if (Qt >Qstar)
		{
			/* code */
			tau = tau * delta;
		}
		if (Qt <Qstar)
		{
			/* code */
			tau = tau - b;
		}
	}



	if (method == 3)
	{

		float eps1 = 0.1;  // delta >1
		//tau = tau - (int) eps*(Qstar- Qt) +(int) eps*(tau -gamma);

		float  lm1 = 1000000000/(float) tau ;
		lm1 = lm1 + eps1*(Qstar- Qt);
		tau = (int) 1000000000/lm1;

	}

	if (method == 4)
	{
		float eps = 0.1;  // delta >1
		float beta = 1;
		//tau = tau - (int) eps*(Qstar- Qt) +(int) eps*(tau -gamma);

		float  lm = 1000000000/(float) tau ;
		lm = lm + eps*(Qstar- Qt)-beta*(lm -1000*gamma);
		tau = (int) 1000000000/lm;


	}
	printf("Tau updated ! %d \n",tau);
	//exit(1);


	if (tau < 10)
	{
		tau = 10;
	}
}


void handle_alarm( int sig ) {

			//int ntau = tau/10;
			alarmCounter = alarmCounter +10 ;
			if (alarmCounter >= tau)
			{
				printf("Tau is %d \n", tau);
				alarmCounter = 0; // reset alarm counter
			
	        char readBuffer[payloadSize];
        	bzero(readBuffer,payloadSize);

	    	rn = read(fd, readBuffer, payloadSize);
    		//if (rn<=0)
    		//	break;

	    	if(rn >0){
    		char sendbackBuffer[payloadSize+4];
    		bzero(sendbackBuffer,payloadSize+4);
    		sprintf(sendbackBuffer,"%04d%s",i,readBuffer);

    		printf("%s\n",sendbackBuffer );
			if(sendto(udpSockFd,sendbackBuffer,strlen(sendbackBuffer),0,(struct sockaddr *)&udpClientAddr,udpClientln) <0){
			printf("Failed to send back\n");	}

			i++;
			//printf("Tau is %d\n",tau );
			//sleep(tau/1000000);
			//usleep(tau%1000000);
			//tau = 1000000;
			//usleep(tau);

			//int r = sleep(10000);   // why sleep does not work?

			//printf("time spend %d\n",r );
		}
	}

}

void myplot(){

    char * commandsForGnuplot[] = {"set title \"Time-lamda\"", "plot 'data.temp'"};
   
    int k =0;

    int allv = 1;
    int ttm = timeArr[0];

   for(k=0;k<t;k++)
    {
    	if(timeArr[k] > ttm){
    		allv++; 
    		ttm = timeArr[k] ;  

    	}

    }

    int xvals[allv] ;
    int yvals[allv] ;
    xvals[0] = 0;
    yvals[0] = 0;
    int ct = 1;
    int tm = timeArr[0];
    int tm0 = timeArr[0];

    for(k=0;k<t;k++)
    {
    	if(timeArr[k] > tm){
    		tm = timeArr[k] ;
    		xvals[ct] =timeArr[k]-tm0;
    		yvals[ct] = lambdaArr[k];
    		ct++;

    	}

    }

	int NUM_COMMANDS = 2;

    FILE * temp = fopen("data.temp", "w");
    /*Opens an interface that one can use to send commands as if they were typing into the
     *     gnuplot command line.  "The -persistent" keeps the plot open even after your
     *     C program terminates.
     */
    FILE * gnuplotPipe = popen ("gnuplot -persistent", "w");
    int i;
    for (i=0; i < allv; i++)
    {
    fprintf(temp, "%d %d \n", xvals[i], yvals[i]); //Write the data to a temporary file
    }

    for (i=0; i < NUM_COMMANDS; i++)
    {
    fprintf(gnuplotPipe, "%s \n", commandsForGnuplot[i]); //Send commands to gnuplot one by one.
    }
	

	}