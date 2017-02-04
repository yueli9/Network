# build an executable named myprog from myprog.c
all: mytunnel tunneld wetalk
mytunnel: mytunnel.c
	gcc -g -Wall -o mytunnel mytunnel.c
tunneld: tunneld.c
	gcc -g -Wall -o tunneld tunneld.c

wetalk: wetalk.c
	gcc -g -Wall -o wetalk wetalk.c
clean:
	$(RM) mytunnel
	$(RM) tunneld
	$(RM) wetalk
