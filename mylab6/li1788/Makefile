# build an executable named myprog from myprog.c
all: filec files traffic_snd traffic_rcv
filec: fileclient.c
	gcc -g -Wall -o filec fileclient.c
files: fileserver.c
	gcc -g -Wall -o files fileserver.c

traffic_snd: traffic_snd.c
	gcc -g -Wall -o traffic_snd traffic_snd.c
traffic_rcv: traffic_rcv.c
	gcc -g -Wall -o traffic_rcv traffic_rcv.c

clean:
	$(RM) filec
	$(RM) files
	$(RM) traffic_snd
	$(RM) traffic_rcv
	$(RM) kline-jarrett.au