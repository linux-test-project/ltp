TARGETS=lftest
CFLAGS+= -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
all: lftest
lftest:
	$(CC) lftest.o -o lftest
install:
	@for i in $(TARGETS) ; do ln -f $$i ../../../bin/$$i ; done
lftest.o: lftest.c
	$(CC) -c lftest.c
clean:
	rm -f *.o lftest
