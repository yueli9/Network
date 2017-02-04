/* mylab6.h */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netdb.h> 
#include <stdbool.h>
#include "mylab6.h"

int dropsendto(int sockFd, const void *message, int length, int flags,
                      const struct sockaddr *dest_addr, socklen_t addrlen, int totalnum, int lossnum){
              
              int result;    
              float ran = ((double)rand() / (double)(RAND_MAX)); 
              float lossrate = (double)lossnum/totalnum;  

            if(ran < 1-lossrate){
              result = sendto(sockFd, message, length, flags, dest_addr, addrlen); 
            }
            else result = 0;

        return  result;        
}
