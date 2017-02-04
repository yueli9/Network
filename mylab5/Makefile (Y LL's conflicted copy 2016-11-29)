# build an executable named audiostreamd audiolisten
all: audiostreamd audiolisten 
audiostreamd: audiostreamd.c
	gcc -g -Wall -o audiostreamd audiostreamd.c -lpthread
audiolisten: audiolisten.c
	gcc -g -Wall -o audiolisten audiolisten.c -lpthread

clean:
	$(RM) audiostreamd
	$(RM) audiolisten