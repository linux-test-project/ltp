TARGETS=lftest
CFLAGS+= -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
all: lftest.o
	gcc lftest.o -o lftest
install: all
	@for i in $(TARGETS) ; do ln -f $$i ../../../bin/$$i ; done
bigftool.o: lftest.c
	gcc -c lftest.c
clean:
	rm -f *.o lftest
