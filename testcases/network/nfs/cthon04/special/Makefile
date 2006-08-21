#
#       @(#)Makefile	1.8 2003/12/29 Connectathon Testsuite
#	1.4 Lachman ONC Test Suite source
#

TESTS=op_unlk op_ren op_chmod dupreq excltest negseek rename holey \
	truncate nfsidem nstat stat stat2 touchn fstat rewind \
	telldir bigfile bigfile2 freesp
DOSRUNFILES = scripts/*.bat
DOSBUILDFILES = console/*.bat console/*.mak dos/*.bat dos/*.mak
DOSFILES = $(DOSRUNFILES) $(DOSBUILDFILES)
DESTDIR=/no/such/path
COPYFILES=runtests runtests.wrk READWIN.txt Makefile

INCLUDES=../tests.h
SUBRS=../basic/subr.o
DEPS=$(INCLUDES) $(SUBRS)

include ../tests.init
CFLAGS := $(shell echo $(CFLAGS))
LIBS := $(shell echo $(LIBS))

all: $(TESTS)
	@chmod a+x runtests

../basic/subr.o:
	cd ../basic; make subr.o

op_unlk: op_unlk.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ $@.c $(LIBS)

op_ren: op_ren.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ $@.c $(LIBS)

op_chmod: op_chmod.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ $@.c $(LIBS)

dupreq: dupreq.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ $@.c $(LIBS)

excltest: excltest.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ $@.c $(LIBS)

negseek: negseek.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ $@.c $(LIBS)

rename: rename.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ $@.c $(LIBS)

holey: holey.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ $@.c $(LIBS)

truncate: truncate.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ $@.c $(LIBS)

nfsidem: nfsidem.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ $@.c $(LIBS)

nstat: nstat.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ $@.c ../basic/subr.o $(LIBS)

stat: stat.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ $@.c ../basic/subr.o $(LIBS)

stat2: stat2.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ $@.c ../basic/subr.o $(LIBS)

touchn: touchn.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ $@.c $(LIBS)

fstat: fstat.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ $@.c $(LIBS)

rewind: rewind.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ $@.c $(LIBS)

telldir: telldir.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ $@.c ../basic/subr.o $(LIBS)

bigfile: bigfile.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ $@.c ../basic/subr.o $(LIBS)

bigfile2: bigfile2.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ $@.c ../basic/subr.o $(LIBS)

freesp: freesp.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ $@.c ../basic/subr.o $(LIBS)

lint:
	lint $(CFLAGS) op_unlk.c
	lint $(CFLAGS) op_ren.c
	lint $(CFLAGS) op_chmod.c
	lint $(CFLAGS) dupreq.c
	lint $(CFLAGS) excltest.c
	lint $(CFLAGS) negseek.c
	lint $(CFLAGS) rename.c
	lint $(CFLAGS) holey.c
	lint $(CFLAGS) truncate.c
	lint $(CFLAGS) nfsidem.c
	lint $(CFLAGS) nstat.c
	lint $(CFLAGS) stat.c
	lint $(CFLAGS) stat2.c
	lint $(CFLAGS) touchn.c
	lint $(CFLAGS) fstat.c
	lint $(CFLAGS) rewind.c
	lint $(CFLAGS) telldir.c
	lint $(CFLAGS) bigfile.c
	lint $(CFLAGS) bigfile2.c
	lint $(CFLAGS) freesp.c

clean:
	rm -f *.o $(TESTS) testfile exctest.file holeyfile

# "copy" can't depend on $(TESTS) because $(TESTS) depends on the
# sources, and the sources might not be available (e.g., running the
# tests from a tree that was created with "make copy").  (runtests
# uses "make copy" to copy the tests over to the server.)
copy:
	cd $(DESTDIR); rm -f $(COPYFILES) $(TESTS)
	cp $(COPYFILES) $(TESTS) $(DESTDIR)

dist:
	cd $(DESTDIR); rm -f $(COPYFILES) *.c $(DOSFILES)
	cp $(COPYFILES) *.c $(DESTDIR)
	tar cf - $(DOSFILES) | (cd $(DESTDIR); tar xfBp -)
