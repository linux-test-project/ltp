#
#       @(#)Makefile	1.9 2003/12/29 Connectathon Testsuite
#
# to make tests, use 'make'
# to copy tests to another directory, use 'make copy DESTDIR=dir'
# to copy source to another directory, use 'make dist DESTDIR=dir'

DESTDIR = /no/such/path

include ../tests.init
CFLAGS := $(shell echo $(CFLAGS))
LIBS := $(shell echo $(LIBS))
LOCKTESTS := $(shell echo $(LOCKTESTS))

LIBS = -lm

all: $(LOCKTESTS)
	@chmod a+x runtests

# Native file offsets (32- or 64-bit), pre-Large File Summit semantics.
tlock:	tlock.c
	$(CC) $(CFLAGS) -o tlock tlock.c $(LIBS)

# Large File Summit semantics for 32- or 64-bit files. 
tlocklfs:	tlock.c
	$(CC) $(CFLAGS) -DLF_SUMMIT -o tlocklfs tlock.c $(LIBS)

# 64-bit file offsets on 32-bit OS.  Assumes Large File Summit
# semantics. 
tlock64:	tlock.c
	$(CC) $(CFLAGS) -DLF_SUMMIT -DLARGE_LOCKS -o tlock64 tlock.c $(LIBS)

clean:
	rm -f *.o tlock tlocklfs tlock64

copy:	all
	cp -f runtests $(LOCKTESTS) $(DESTDIR)

dist:
	cp -f runtests Makefile *.c $(DESTDIR)

lint: tlock.c
	lint $(CFLAGS) tlock.c $(LIBS)

lint32lfs: tlock.c
	lint $(CFLAGS) -DLF_SUMMIT tlock.c $(LIBS)

lint64: tlock.c
	lint $(CFLAGS) -DLF_SUMMIT -DLARGE_LOCKS tlock.c $(LIBS)

lintall: lint lint32lfs lint64
