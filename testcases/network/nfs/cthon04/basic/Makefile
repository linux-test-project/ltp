#
#	@(#)Makefile	1.8 2003/12/29 Connectathon Testsuite
#	1.5 Lachman ONC Test Suite source
#
# to make tests, use 'make'
# to copy tests to another directory, use 'make copy DESTDIR=dir'
# to copy source to another directory, use 'make dist DESTDIR=dir'

TESTS = test1 test2 test3 test4 test5 test6 test7 test8 test9
AUXTESTS = test4a test5a test5b test7a test7b
DOSRUNFILES = scripts/*.bat scripts/*.mst scripts/*.prl
DOSBUILDFILES = console/*.bat console/*.mak dos/*.bat dos/*.mak
DOSFILES = $(DOSRUNFILES) $(DOSBUILDFILES)
DESTDIR = /no/such/path

INCLUDES=../tests.h

include ../tests.init
CFLAGS := $(shell echo $(CFLAGS))
LIBS := $(shell echo $(LIBS))

all: origtests auxtests
	@chmod a+x runtests

origtests: $(TESTS)
auxtests: $(AUXTESTS)

subr.o:	subr.c $(INCLUDES)

test1: test1.c $(INCLUDES) subr.o
	$(CC) $(CFLAGS) -o $@ $@.c subr.o $(LIBS)

test2: test2.c $(INCLUDES) subr.o
	$(CC) $(CFLAGS) -o $@ $@.c subr.o $(LIBS)

test3: test3.c $(INCLUDES) subr.o
	$(CC) $(CFLAGS) -o $@ $@.c subr.o $(LIBS)

test4: test4.c $(INCLUDES) subr.o
	$(CC) $(CFLAGS) -o $@ $@.c subr.o $(LIBS)

test4a: test4a.c $(INCLUDES) subr.o
	$(CC) $(CFLAGS) -o $@ $@.c subr.o $(LIBS)

test5: test5.c $(INCLUDES) subr.o
	$(CC) $(CFLAGS) -o $@ $@.c subr.o $(LIBS)

test5a: test5a.c $(INCLUDES) subr.o
	$(CC) $(CFLAGS) -o $@ $@.c subr.o $(LIBS)

test5b: test5b.c $(INCLUDES) subr.o
	$(CC) $(CFLAGS) -o $@ $@.c subr.o $(LIBS)

test6: test6.c $(INCLUDES) subr.o
	$(CC) $(CFLAGS) -o $@ $@.c subr.o $(LIBS)

test7: test7.c $(INCLUDES) subr.o
	$(CC) $(CFLAGS) -o $@ $@.c subr.o $(LIBS)

test7a: test7a.c $(INCLUDES) subr.o
	$(CC) $(CFLAGS) -o $@ $@.c subr.o $(LIBS)

test7b: test7b.c $(INCLUDES) subr.o
	$(CC) $(CFLAGS) -o $@ $@.c subr.o $(LIBS)

test8: test8.c $(INCLUDES) subr.o
	$(CC) $(CFLAGS) -o $@ $@.c subr.o $(LIBS)

test9: test9.c $(INCLUDES) subr.o
	$(CC) $(CFLAGS) -o $@ $@.c subr.o $(LIBS)

lint:
	lint $(CFLAGS) test1.c subr.c $(LIBS)
	lint $(CFLAGS) test2.c subr.c $(LIBS)
	lint $(CFLAGS) test3.c subr.c $(LIBS)
	lint $(CFLAGS) test4.c subr.c $(LIBS)
	lint $(CFLAGS) test4a.c subr.c $(LIBS)
	lint $(CFLAGS) test5.c subr.c $(LIBS)
	lint $(CFLAGS) test5a.c subr.c $(LIBS)
	lint $(CFLAGS) test5b.c subr.c $(LIBS)
	lint $(CFLAGS) test6.c subr.c $(LIBS)
	lint $(CFLAGS) test7.c subr.c $(LIBS)
	lint $(CFLAGS) test7a.c subr.c $(LIBS)
	lint $(CFLAGS) test7b.c subr.c $(LIBS)
	lint $(CFLAGS) test8.c subr.c $(LIBS)
	lint $(CFLAGS) test9.c subr.c $(LIBS)

clean:
	rm -f *.o $(TESTS) $(AUXTESTS)

copy: $(TESTS)
	cp -f runtests runtests.mvs $(TESTS) $(AUXTESTS) $(DESTDIR)

dist:
	cp -f runtests runtests.mvs Makefile *.c $(DESTDIR)
	tar cf - $(DOSFILES) | (cd $(DESTDIR); tar xfBp -)
