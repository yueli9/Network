/* Raw mode demo */
/* See exactly what is being transmitted from the terminal. To do this
   we have to be more careful. */

#include <stdio.h>
#include <signal.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>

struct termios oldtermios;

int ttyraw(int fd)
{

	struct termios newtermios;
	if(tcgetattr(fd, &oldtermios) < 0)
		return(-1);
	newtermios = oldtermios;

	newtermios.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

	newtermios.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

	newtermios.c_cflag &= ~(CSIZE | PARENB);


	newtermios.c_cflag |= CS8;

	newtermios.c_oflag &= ~(OPOST);

	newtermios.c_cc[VMIN] = 1;
	newtermios.c_cc[VTIME] = 0;

	if(tcsetattr(fd, TCSAFLUSH, &newtermios) < 0)
		return(-1);
	return(0);
}
	
	 
int ttyreset(int fd)
{
	if(tcsetattr(fd, TCSAFLUSH, &oldtermios) < 0)
		return(-1);

	return(0);
}

void sigcatch(int sig)
{
	ttyreset(0);
	exit(0);
}

void main()
{
	int i;
	char c;

/*
	if((int) signal(SIGINT,sigcatch) < 0)
	{
		perror("signal");
		exit(1);
	}
	if((int)signal(SIGQUIT,sigcatch) < 0)
	{
		perror("signal");
		exit(1);
	}
	if((int) signal(SIGTERM,sigcatch) < 0)
	{
		perror("signal");
		exit(1);
	}
*/
	/* Set raw mode on stdin. */
	if(ttyraw(0) < 0)
	{
		fprintf(stderr,"Can't go to raw mode.\n");
		exit(1);
	}

	char buf[1024];
	bzero(buf,1024);
	int t=0; 
		while( (i = read(0, &c, 1)) == 1)
	{
		buf[t]=c;
		t++;
		if( (c &= 255) == 015) /* ASCII DELETE */
			break;
		printf( "%c\n\r", c);

		//printf( "%c\n\r", c);
	}
	
	if(ttyreset(0) < 0)
	{
		fprintf(stderr, "Cannot reset terminal!\n");
		exit(-1);
	}
	printf("%s\n", buf);

	if( i < 0)
	{
		fprintf(stderr,"Read error.\n");
		exit(-1);
	}

	exit(0);
}
