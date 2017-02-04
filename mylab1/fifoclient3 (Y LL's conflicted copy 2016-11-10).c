#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
        
#define MAX_BUFFER_SIZE             8192
#define CMD_FIFO_NAME               "cmdfifo"
#define CLIENT_FIFO_NAME            "cfifo%u"

// change this definition if you want to test other commands
#define CMD_TO_SEND                 "ls"


int main(int argc, char *argv[])
{
    pid_t mypid = getpid();

    char client_fifo_name[MAX_BUFFER_SIZE] = "";
    sprintf(client_fifo_name, CLIENT_FIFO_NAME, mypid);
    
    // check command length
    if (strlen(CMD_TO_SEND) + 10 > MAX_BUFFER_SIZE) {
        fprintf(stderr, "Error: command length is too long.");
        return -1;
    }

    // create fifo
    if (mkfifo(client_fifo_name, 0666) < 0) {
        perror("Error: mkfifo: ");
        return -1;
    }
    
    // open cmd fifo
    int fd_cmd = open(CMD_FIFO_NAME, O_WRONLY);
    if (fd_cmd < 0) {
        perror("Error: open: ");
        return -1;
    }

    int fd_resp = -1;
    char buf[MAX_BUFFER_SIZE] = "";
    
    // write command
    sprintf(buf, "$%u$%s", mypid, argv[1]); 
    //sprintf(buf, "$%u$%s", mypid, CMD_TO_SEND); 
    ssize_t wlen = write(fd_cmd, buf, strlen(buf));
    //close(fd_cmd);
    if (wlen != strlen(buf)) {
        perror("Error: write: ");
    }
    else {
        // block
        // open response fifo
        fd_resp = open(client_fifo_name, O_RDONLY);
        if (fd_resp < 0) {
            perror("Error: open: ");
            return -1;
        }
        
        // read response
        ssize_t rlen = 0;
        while ((rlen = read(fd_resp, buf, MAX_BUFFER_SIZE-1)) != 0) {
            buf[rlen] = '\0';
            printf("%s", buf);
        }
        printf("\n");
    }

    close(fd_cmd);
    close(fd_resp);
}

