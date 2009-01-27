#
# Makefile for diotests
#
# History:
# 04/10/2002	nsharoff	nsharoff@us.ibm.com
#


CFLAGS+=		 -O -Wall -DSHARED_OFILE -D_GNU_SOURCE -I../../../../include -L../../../../lib 
LOADLIBES+= -lltp

SRCS=	diotest1.c diotest2.c diotest3.c diotest4.c diotest5.c \
	diotest6.c dma_thread_diotest7.c

BINS=	diotest1 diotest2 diotest3 diotest4 diotest5 \
	diotest6 dma_thread_diotest7  

all:	diotest_routines.o bins

bins: $& $(BINS)

diotest_routines.o: diotest_routines.c
		 $(CC) -c diotest_routines.c $(CFLAGS) 

diotest1: diotest1.c diotest_routines.o
	$(CC) -o diotest1 diotest_routines.o diotest1.c  $(CFLAGS) $(LDFLAGS) $(LOADLIBES)
	rm -f diotest1.o

diotest2: diotest2.c diotest_routines.o
	$(CC) -o diotest2 diotest_routines.o diotest2.c $(CFLAGS) $(LDFLAGS) $(LOADLIBES)
	rm -f diotest2.o

diotest3: diotest3.c diotest_routines.o 
	$(CC) -o diotest3 diotest_routines.o diotest3.c $(CFLAGS) $(LDFLAGS) $(LOADLIBES)
	rm -f diotest3.o

diotest4: diotest4.c diotest_routines.o
	$(CC) -o diotest4 diotest_routines.o diotest4.c $(CFLAGS) $(LDFLAGS) $(LOADLIBES)
	rm -f diotest4.o

diotest5: diotest5.c diotest_routines.o
	$(CC) -o diotest5 diotest_routines.o diotest5.c $(CFLAGS) $(LDFLAGS) $(LOADLIBES)
	rm -f diotest5.o

diotest6: diotest6.c diotest_routines.o
	$(CC) -o diotest6 diotest_routines.o diotest6.c $(CFLAGS) $(LDFLAGS) $(LOADLIBES)
	rm -f diotest6.o

dma_thread_diotest7: dma_thread_diotest7.c
	$(CC) -g -o dma_thread_diotest7 dma_thread_diotest7.c -lpthread

clean:	
	rm -f $(BINS) diotest_routines.o

install:
	ln -f $(BINS) ../../../bin
	ln -f test_dma_thread_diotest7.sh ../../../bin/

