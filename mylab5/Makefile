# build an executable named audiostreamd audiolisten
all: audiostreamd audiolisten 
audiostreamd: audiostreamd.c
	gcc -g -Wall -o audiostreamd audiostreamd.c
audiolisten: audiolisten.c
	gcc -g -Wall -o audiolisten audiolisten.c

clean:
	$(RM) audiostreamd
	$(RM) audiolisten