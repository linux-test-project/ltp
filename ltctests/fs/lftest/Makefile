TARGETS=lftest
CFLAGS+= -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
all: lftest.o
	gcc lftest.o -o lftest
install: all
	cp -a $(TARGETS) ../../bin
bigftool.o: lftest.c
	gcc -c lftest.c
clean:
	rm -f *.o lftest
