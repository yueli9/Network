# build an executable named myprog from myprog.c
all: fifoc fifos mcs mucs fifoc2 fifos2 
fifoc: fifoclient.c
	gcc -g -Wall -o fifoc fifoclient.c
fifos: fifoserver.c
	gcc -g -Wall -o fifos fifoserver.c
mcs: mychecksum.c
	gcc -g -Wall -o mcs mychecksum.c
mucs: myunchecksum.c
	gcc -g -Wall -o mucs myunchecksum.c
fifoc2: fifoclient2.c
	gcc -g -Wall -o fifoc2 fifoclient2.c
fifos2: fifoserver2.c
	gcc -g -Wall -o fifos2 fifoserver2.c

clean:
	$(RM) fifoc
	$(RM) fifos
	$(RM) mcs
	$(RM) fifoc2
	$(RM) fifos2
	$(RM) mucs
