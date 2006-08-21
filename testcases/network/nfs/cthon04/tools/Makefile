#
#       @(#)Makefile	1.3 99/11/19 Connectathon Testsuite
#	1.5 Lachman ONC Test Suite source
#
#
# 'make all'                    makes test programs
# 'make clean'                  cleans directories
# 'make copy DESTDIR=path'      copies test programs to path
# 'make dist DESTDIR=path'      copies sources to path

TESTS = tcp tcpd udp udpd dirdmp dirprt pmaptst pmapbrd

# dirdmp apparently doesn't work with Linux 2.2.  So for Linux systems,
# comment out the above definition and use this one.
#TESTS = tcp tcpd udp udpd dirprt pmaptst pmapbrd

include ../tests.init
CFLAGS := $(shell echo $(CFLAGS))
LIBS := $(shell echo $(LIBS))

all: $(TESTS)

tcp: tcp.c
	$(CC) $(CFLAGS) -o $@ $@.c $(LIBS)
tcpd: tcpd.c
	$(CC) $(CFLAGS) -o $@ $@.c $(LIBS)
udp: udp.c
	$(CC) $(CFLAGS) -o $@ $@.c $(LIBS)
udpd: udpd.c
	$(CC) $(CFLAGS) -o $@ $@.c $(LIBS)
dirdmp: dirdmp.c
	$(CC) $(CFLAGS) -o $@ $@.c $(LIBS)
dirprt: dirprt.c
	$(CC) $(CFLAGS) -o $@ $@.c $(LIBS)
pmaptst: pmaptst.c
	$(CC) $(CFLAGS) -o $@ $@.c $(LIBS)
pmapbrd: pmapbrd.c
	$(CC) $(CFLAGS) -o $@ $@.c $(LIBS)

lint:
	lint $(CFLAGS) tcp.c
	lint $(CFLAGS) tcpd.c
	lint $(CFLAGS) udp.c
	lint $(CFLAGS) udpd.c
	lint $(CFLAGS) dirdmp.c
	lint $(CFLAGS) dirprt.c
	lint $(CFLAGS) pmaptst.c
	lint $(CFLAGS) pmapbrd.c

clean:
	rm -f *.o $(TESTS)

copy: $(TESTS)
	cp -f $(TESTS) $(DESTDIR)

dist:
	cp -f Makefile README *.c $(DESTDIR)
