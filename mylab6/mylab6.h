#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>


//#ifndef MYLAB6_H_
//#define MYLAB6_H_


int dropsendto(int sockFd, const void *message, int len, int flags,
                      const struct sockaddr *dest_addr, socklen_t addrlen, int totalnum, int lossnum);


//#endif /* MYLAB6_H_ */


