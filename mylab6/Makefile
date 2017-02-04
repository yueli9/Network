# build an executable named audiostreamd audiolisten
all: overlayrouter overlaybuild  turboclient turboserver
overlayrouter: overlayrouter.c
	gcc -g -Wall -o overlayrouter overlayrouter.c
overlaybuild: overlaybuild.c
	gcc -g -Wall -o overlaybuild overlaybuild.c
turboclient: turboclient.c
	gcc -g -Wall -o turboclient -include mylab6.h turboclient.c dropsendto.c 
turboserver: turboserver.c
	gcc -g -Wall -o turboserver -include mylab6.h turboserver.c dropsendto.c 

clean:
	$(RM) overlayrouter
	$(RM) overlaybuild
	$(RM) turboclient
	$(RM) turboserver