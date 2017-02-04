//mychecksum
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <stdint.h>

int main(int argc, char **argv)
{
uint64_t cksm;
char buf;
unsigned int bb;

int f1  =open(argv[1],O_RDONLY);
int f2  =open(argv[2],O_CREAT|O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);

cksm =0;
bb =read(f1, &buf, 1); 	//read one byte
while(bb >0 ){		//until EOF
write(f2,&buf,1);	//write byte to f2
cksm = cksm + (int)buf;	// sum up
bb =read(f1, &buf, 1);	// read next byte
}
uint64_t be;
int num =1;
if(*(char *)&num == 1){ 
printf("current little endian\n");
be =  htobe64(cksm);
}
else{
printf("current big endian\n");
be = cksm;
}

printf("raw checksum %llx \n",cksm);
printf("be checksum %llx \n",be);

char msg[8];
int i;
//for(i = 0; i < 8; i++) 
//	msg[i] = be >> (8-1-i)*8;
//	printf("%s\n",msg[i]);
//	write(f2,msg[i],1);

printf("file1 last 8:\n");

unsigned char *p = (char*)&be;
for(i=0;i<8;i++){
write(f2,&(p[i]),1);

printf("%02x\n",(unsigned int)(unsigned char)(p[i]));
}

/*
unsigned char* q = (char*)&cksm;
for(i=0;i<8;i++){
//write(f2,&(q[i]),1);
printf("%x\n",(q[i]));
}
*/

//write(f2,be,8);
close(f1);
close(f2);

return 1;
}
