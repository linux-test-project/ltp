CFLAGS=-Wall -pedantic -std=c99 -O2 -g  `getconf LFS_CFLAGS` -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE

all: writetest.c

install:
	ln -f writetest ../../../bin

clean:
	$(RM) -f writetest
