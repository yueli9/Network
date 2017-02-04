// simple UDP ping server

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <signal.h>

/*
void handle_alarm( int sig ) {
    
        printf("no response from real server skip\n");
        continue;
   
}

*/

// create socket for receiving from client
int createSocket(int portNum){

int tsockFd;
struct sockaddr_in svaddr;
memset(&svaddr, 0, sizeof(svaddr));

svaddr.sin_family = AF_INET;
svaddr.sin_addr.s_addr = INADDR_ANY;
svaddr.sin_port = htons(portNum);

tsockFd = socket(AF_INET,SOCK_DGRAM,0);

if(tsockFd <0){
	printf("ERROR cereating socket\n");
    exit(1);
}

if(bind(tsockFd,(struct sockaddr *) &svaddr, sizeof(svaddr)) <0){
	printf("ERROR binding socket\n");
	exit(1);
}

return tsockFd;
}

// create socket for sending to server


int connectSocket(char *intAddr,int PortNum,struct sockaddr_in *svaddr){
	//struct sockaddr_in svaddr;
	int sockFd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockFd <0){       
        printf("ERROR openning socket");
        exit(1);
    }

    memset(&(*svaddr), 0, sizeof(*svaddr));

    svaddr->sin_family = AF_INET;
    svaddr->sin_addr.s_addr = inet_addr(intAddr);
    svaddr->sin_port = htons(PortNum);

    if (connect(sockFd,(struct sockaddr *) &(*svaddr),sizeof(*svaddr)) < 0) {       
        printf("ERROR creating socket\n");
        exit(1);
    }

    printf("success create socket to real server\n");
    return sockFd;

}



int getlast(char *line,char **result,char **secdLast){

    //char line[30] = "I need to see this in reverse";


    char *tokens[30]; 
    int max_tokens = sizeof(tokens)/sizeof(tokens[0]); 
    char *token_ptr; 
    int i; 

    token_ptr = strtok(line, "$");
    for (i = 0; i < max_tokens && token_ptr != NULL; i++) {
        tokens[i] = token_ptr; 
        printf("tokens[%d]: <%s>\n", i, tokens[i]); 
        token_ptr = strtok(NULL, "$"); 
       }

    //printf("Get the result %d\n", i);

    *result = malloc(sizeof(char) * strlen(tokens[i-1]));
       // printf("result: %s\n",*result );

    strcpy(*result,tokens[i-1]);
    //*result = tokens[i-1];
    printf("result: %s\n",*result );

    
    if (i>1)
    {

    *secdLast = malloc(sizeof(char) * strlen(tokens[i-2]));
    strcpy(*secdLast,tokens[i-2]);

    //*secdLast = tokens[i-2];
    printf("secdLast %s\n",*secdLast );

    sprintf(line,"$%s",tokens[0]);
    printf("line: %s\n",line );

    int j = 0;
    for ( j = 1; j < i-1; j++)
    {
        sprintf(line,"%s$%s",line,tokens[j]);
            printf("line: %s\n",line );

    }
    sprintf(line,"%s$",line);
            printf("line: %s\n",line );

    }

    return 1;

    }


int isIPadd( char *String){
    int i = 0;
    int strl = strlen(String);
    for ( i = 0; i < strl; i++)
    {
        if (String[i] == '.')
        {
            return 1;         
        }
    }
    return 0;

}

int isLast( char *String){
    int i = 0;
    int count =0;
   int strl = strlen(String);
    for ( i = 0; i < strl; i++)
    {
        if (String[i] == '$')
        {
            count++ ;         
        }
    }

    if (count ==3)
    {
       return 1;
    }
    else
        return 0;

}

bool lastVPN ;

int main(int argc, char *argv[])
{


struct sockaddr_in rvAddr[5];
socklen_t rvSize = sizeof(rvAddr[0]);

int status;
int len;
char *token;
char severIP[50];
bzero(severIP,50);

char currentIP[50];
bzero(currentIP,50);

int serverPtNum =0;

char Sseckey[20] = "ddmt7OAOOxMLa";
char buf[1000];
bzero(buf,1000);

char tbuf[1024];
bzero(tbuf,1024);

char sbuf[1000] ;
bzero(sbuf,1000);

struct sockaddr_storage sStor,rvStore;

struct sockaddr_storage clientStore[5],serverStore[5];

socklen_t addrSize = sizeof(sStor);

 	char serverIP[20];  // actual server IP received
    bzero(serverIP,20);

    char serverPort[10];      //second vpn port number from vpn server
    bzero(serverPort,10);


if (argc < 2) {
     printf("ERROR,overlayrouter server-port\n");
     exit(1);
 }

int i =0;


int sockFd = createSocket(atoi(argv[1]));// create vpn socket;
int svPt[5] ={57343,56893,59283,59334,53945};
int svSocket[5];
bool svAvail[5];
int rsvPort[5];
char rsvIP[5][48];
int k = 0;
int realsvSocket[5];
int datasvSocket[5];

struct sockaddr_in realServAddr[5];   // store address for real server
struct sockaddr_in realServStore[5];   // store address for real server
struct sockaddr_in dataServAddr[5];   // store address for real server

//struct sockaddr_in vaddr
// initialized the struct
for (k =0;k<5;k++){
	svSocket[k] = createSocket(svPt[k]);	// create socket for real client
	svAvail[k] = true;
    rsvPort[k] = 0;
    bzero(rsvIP[k],48);
    realsvSocket[k] = 0;
    datasvSocket[k] =0;
}

int tn = 4;

char    sev_ip[1024] = "";

//inet_ntop(AF_INET, &svaddr.sin_addr, sev_ip, sizeof(sev_ip));
//printf("server ip: %s Port Number %d \n",sev_ip,ntohs(svaddr.sin_port));

//ctN = sizeof(ctaddr);

//unsigned char *ip = svaddr.sin_addr.s_addr;
//printf("Received From: %d.%d.%d.%d .\n",ip[0], ip[1], ip[2], ip[3]);

while(1) {
    //printf("Waiting for new msg\n");
	int nb = recvfrom(sockFd,buf,1000,MSG_DONTWAIT,(struct sockaddr *)&rvStore, &addrSize);
   	
    if(nb >0){

        printf("Received %s \n",buf);
/*
    char result[100];
    bzero(result,100);
    char secdLast[100];
    bzero(secdLast,100);
*/
       char * result;
       char * secdLast; 
  
    getlast( buf, &result,&secdLast);

    printf("buf %s result %s secdLast %s\n",buf,result,secdLast );

    int NTsockFd;                       // semd  msg to next server.
    struct sockaddr_in NTsvaddr;
    memset(&NTsvaddr, 0, sizeof(NTsvaddr));
    int srl;


    if (isLast(buf) == 1) // this is the last router
    {
        lastVPN = true;
        strcpy(currentIP,result);


        printf("currentIP %s\n",currentIP );
        //getlast( buf, result,secdLast);

        char *token;
        //bzero(token,50);
        token = strtok(buf, "$");
        strcpy(severIP,token);  // next server number: the actual server
         token = strtok(NULL, "$"); 

        //strcpy(severIP, secdLast);
        printf("toek is %s\n",token );
        serverPtNum = atoi(token);  // next port number
    }

    // otherwise transit router or first router
    else{
        lastVPN= false;
        strcpy(currentIP, result);
        strcpy(severIP, secdLast);
        serverPtNum = atoi(argv[1]);

    }

                //fd_set rfds;

                //int retval;

                //FD_ZERO(&rfds);

    // allocate an port number for this new transit

	for (k =0;k<5;k++){
	if (svAvail[k] == true)
		{
			 int ln = sprintf(sbuf,"$%s$%d$",currentIP,svPt[k]);
             sbuf[ln] ='\0';
             printf("send to vpn client : %s  length:%d\n",sbuf,strlen(sbuf));
			 int sbs = sendto(sockFd,sbuf,strlen(sbuf),0,(struct sockaddr *)&rvStore,addrSize);
             if(sbs <0){
                printf("Failed to send back to vpn client %d \n",sbs);
                exit(0);
             }
             printf("send back size %d \n",sbs);
		     bzero(sbuf,100);

            if(lastVPN== false){        // not last VPN

             rsvPort[k] = serverPtNum;//atoi(token);
             strcpy(rsvIP[k] , severIP);
             realsvSocket[k]  = connectSocket(severIP,serverPtNum,&realServAddr[k]); // connect to next VPN server
             svAvail[k] = false;


            struct timeval tv;

            tv.tv_sec = 30;  /* 30 Secs Timeout */
            tv.tv_usec = 0;  // Not init'ing this can cause strange errors

            setsockopt(realsvSocket[k], SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));  // set 3s timeout

			printf("The %d Port Number is assigned\n",k);

            //if(lastVPN== false){        // not last VPN

                int srl =sendto(realsvSocket[k],buf,strlen(buf) ,0,(struct sockaddr_in *)&realServAddr[k],sizeof(realServAddr[k]));  // send to NXT server
                if(srl <0){
                printf("ERROR sending to real UDP server\n");
                exit(1);
                }


                char rsbuf[1024];
                bzero(rsbuf,1024);

                socklen_t tmpSize = sizeof(realServAddr[k]);

                int rsbl = recvfrom(realsvSocket[k],rsbuf,1024,0,(struct sockaddr *)&(realServAddr[k]), &tmpSize);

                if(rsbl <0){
                printf("TIMEOUT receiving from next VPN server\n");  // timeout after 30 s
                svAvail[k] = true;
                }

                printf("Received from next VPN %s\n",rsbuf );

                char nxtIP[50];
                bzero(nxtIP,50);
                char nxtPort[50];
                bzero(nxtPort,50);
                char *token2;
                token2 = strtok(rsbuf, "$");
                printf("token2%s\n",token2 );
                strcpy(nxtIP,token2);
                token2 = strtok(NULL, "$"); 
                printf("token2: %s\n",token2 );
                strcpy(nxtPort,token2);
                printf("nxtIP %s nxtPort %s\n",nxtIP,nxtPort );

                datasvSocket[k]  = connectSocket(nxtIP,atoi(nxtPort),&dataServAddr[k]); // connect to next VPN server via DATA Port

                struct timeval dtv;

                dtv.tv_sec = 0;  /* 30 Secs Timeout */
                dtv.tv_usec =5;  // Not init'ing this can cause strange errors

                setsockopt(datasvSocket[k], SOL_SOCKET, SO_RCVTIMEO, (char *)&dtv,sizeof(struct timeval));  // set 3s timeout



                //FD_SET(0, &rfds);
               // FD_SET(datasvSocket[k], &rfds);



                //retval = select(datasvSocket[k] +1, &rfds, NULL, NULL, NULL);   // waiting for stdin and recvsockfd



















            }

            else{
                printf("Last VPN\n");
                printf("next IP %s Port %d \n",severIP,serverPtNum );
                datasvSocket[k]  = connectSocket(severIP,serverPtNum,&dataServAddr[k]); // connect to real server via real Port
                svAvail[k] = false;
                struct timeval ndtv;

                ndtv.tv_sec = 0;  /* 30 Secs Timeout */
                ndtv.tv_usec = 10;  // Not init'ing this can cause strange errors

                setsockopt(datasvSocket[k], SOL_SOCKET, SO_RCVTIMEO, (char *)&ndtv,sizeof(struct timeval));  // set 3s timeout


            }

            break;


		}

	}

    }

    else{           // 

    for (i =0;i<5;i++){
        if(  svAvail[i] == false){
        
        char dtbuf[2048];
        bzero(dtbuf,2048);

        int dtnb = recvfrom(svSocket[i],dtbuf,2048,0,(struct sockaddr *)&clientStore[i], &addrSize);
        if(dtnb <0){
        printf("ERROR receiving from UDP socket\n");
        exit(1);
            }
        if(dtnb == 0)
            continue;

        printf("buf length %d,%s\n ",dtnb,dtbuf);

        int dsrl =sendto(datasvSocket[i],dtbuf,strlen(dtbuf) ,0,(struct sockaddr_in *)&dataServAddr[i],sizeof(dataServAddr[i]));  // send to real server
        if(dsrl <0){
            printf("ERROR sending to real UDP server\n");
            exit(1);
        }
        
        char drsbuf[1024];
        bzero(drsbuf,1024);


        socklen_t dtmpSize = sizeof(dataServAddr[i]);

        printf("waiting for response from next server\n");
        //int rsbl = recvfrom(realsvSocket[i],rsbuf,1024,0,(struct sockaddr_in *)&realServAddr[i], &tmpSize);

        //signal( SIGALRM, handle_alarm ); // Install handler first,


        int drsbl = recvfrom(datasvSocket[i],drsbuf,1024,0,(struct sockaddr *)&(dataServAddr[i]), &dtmpSize);



        if(drsbl <0){
        printf("TIMEOUT receiving from next UDP server\n");  // timeout after 1s
        //exit(1);
        continue;
            }
        if(drsbl == 0)
            continue;

        printf("Receive from next server,buf length %d,%s\n ",drsbl,drsbuf);                   // send to client

        int rcbl =sendto(svSocket[i],drsbuf,strlen(drsbuf) ,0,(struct sockaddr_in *)&clientStore[i],sizeof(clientStore[i]));
        if(rcbl <0){
        printf("ERROR sending to next UDP client\n");
        exit(1);
            }

        //printf("VPN circle finish \n");

        }
        //printf("Outer VPN circle finish \n");

        }
	//printf("success sent back!\n");

    }

  }

close(sockFd);
return 0;
}
