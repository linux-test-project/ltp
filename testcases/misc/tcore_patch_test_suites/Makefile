CC=gcc
MODCFLAGS := -Wall -DLINUX -I/usr/src/linux/include
EXECS=tcore tcore.sh tcore.exp

all:		tcore

install: 
		@set -e; for i in $(EXECS) ; do ln -f $$i ../../bin/$$i ; done ;	

tcore:		tcore.c
		$(CC)  -o tcore tcore.c -lm -lpthread
clean:
	rm -f tcore

