CFLAGS=-Wall -O2 -g  `getconf LFS_CFLAGS` -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE

all: writetest

writetest:
	$(CC) $(CFLAGS) writetest.c -o writetest

install:
	ln -f writetest ../../../bin

clean:
	$(RM) -f writetest
