#
#       @(#)Makefile	1.5 2003/12/29 Connectathon Testsuite
#	1.3 Lachman ONC Test Suite source
#

DESTDIR=/no/such/path
FILES=Makefile runtests runtests.wrk *.sh *.c mkdummy rmdummy \
	nroff.in makefile.tst

LARGE_SRC=large1.c large2.c large3.c

all: $(LARGE_SRC)
	@chmod a+x runtests

large1.c: large.c
	rm -f large1.c
	cp large.c large1.c

large2.c: large.c
	rm -f large2.c
	cp large.c large2.c

large3.c: large.c
	rm -f large3.c
	cp large.c large3.c

lint:

clean:
	rm -f *.time *.o stat $(LARGE_SRC)

copy dist: all
	cd $(DESTDIR); rm -f $(FILES)
	cp $(FILES) $(DESTDIR)
