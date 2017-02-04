//myunchecksum
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
int f1;
int f2  =open(argv[1],O_RDONLY);
int NoB = 0;
cksm =0;
while(read(f2, &buf, 1)>0){ 	//read one byte
NoB++;
}			// record number of bytes in f2
close(f2);			// close f2


int f2b  =open(argv[1],O_RDONLY); //open f2 again
char bufA[NoB-8]; // store character array
int i;
printf("length of file %d\n",NoB);
for(i=0;i<NoB-8;i++)
{				//until EOF
bb =read(f2b, &buf, 1);		// read next byte
cksm = cksm + (int)buf;		// sum up
bufA[i] =buf; 			//store byte to bufA
}

int len = strlen(bufA);
bufA[len-1] = '\0';

unsigned char rcs[8];		// record last 8 byte
int j;
for(j=0;j<8;j++){
read(f2b, &buf, 1);
rcs[7-j]=buf;			// reverse to get small endian
printf("buf :%x \n",buf);
}

printf("Checksum signature is: 0x");
for(j=0;j<8;j++){
printf("%02x",rcs[j]);
}
printf("\n");


printf("New file2 checksum is: 0x");
unsigned char* q = (char*)&cksm;
for(i=0;i<8;i++){
//write(f2,&(q[i]),1);
printf("%02x",(q[i]));
}
printf("\n");

int match = 0;
for(i=0;i<8;i++){
if(q[i] != rcs[i])
{
match = 1;
}
}

if(1 == match){
printf("Checksum No Match,the the file  has been corrupted\n");
f1  =open(argv[2],O_CREAT|O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
write(f1,&bufA,NoB-8);
}
else
{
printf("The two files' checksums match. \n");
}


close(f1);
close(f2b);
return 1;
}
