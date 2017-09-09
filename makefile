CFLAGS=`pkg-config --cflags gtk+-3.0 libvlc`
LIBS=`pkg-config --libs gtk+-3.0 libvlc`

all:
	gcc -o simpleVideoPlayer simpleVideoPlayer.c -Wall ${LIBS} ${CFLAGS}
clean:
	rm simpleVideoPlayer
