#
# Makefile for diotests
#
# History:
# 04/10/2002	nsharoff	nsharoff@us.ibm.com
#


CFLAGS+=		 -O -DSHARED_OFILE -D_GNU_SOURCE

SRCS=	diotest1.c diotest2.c diotest3.c diotest4.c diotest5.c \
	diotest6.c

BINS=	diotest1 diotest2 diotest3 diotest4 diotest5 \
	diotest6

all:	diotest_routines.o bins

bins: $& $(BINS)

diotest_routines.o: diotest_routines.c
	$(CC) $(CFLAGS) -c diotest_routines.c

diotest1: diotest1.c diotest_routines.o
	$(CC) $(CFLAGS) -o diotest1 diotest1.c diotest_routines.o
	rm -f diotest1.o

diotest2: diotest2.c diotest_routines.o
	$(CC) $(CFLAGS) -o diotest2 diotest2.c diotest_routines.o
	rm -f diotest2.o

diotest3: diotest3.c diotest_routines.o 
	$(CC) $(CFLAGS) -o diotest3 diotest3.c diotest_routines.o
	rm -f diotest3.o

diotest4: diotest4.c diotest_routines.o
	$(CC) $(CFLAGS) -o diotest4 diotest4.c diotest_routines.o
	rm -f diotest4.o

diotest5: diotest5.c diotest_routines.o
	$(CC) $(CFLAGS) -o diotest5 diotest5.c diotest_routines.o
	rm -f diotest5.o

diotest6: diotest6.c diotest_routines.o
	$(CC) $(CFLAGS) -o diotest6 diotest6.c diotest_routines.o
	rm -f diotest6.o

t3: t3.c 
	$(CC) $(CFLAGS) -o t3 t3.c
	rm -f t3.o

clean:	
	rm -f $(BINS) diotest_routines.o
	cd ../../../bin
	rm -f $(BINS)

install:
	ln -f $(BINS) ../../../bin

