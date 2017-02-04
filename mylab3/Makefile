# build an executable named myprog from myprog.c
all: mytunnel tunneld
filec: mytunnel.c
	gcc -g -Wall -o mytunnel mytunnel.c
files: fileserver.c
	gcc -g -Wall -o tunneld tunneld.c

clean:
	$(RM) mytunnel
	$(RM) tunneld
