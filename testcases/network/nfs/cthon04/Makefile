#
#       @(#)Makefile	1.6 2003/12/29 Connectathon Testsuite
#
# 'make all'			makes test programs
# 'make clean'			cleans directories
# 'make copy DESTDIR=path'	copies test programs to path
# 'make dist DESTDIR=path'	copies sources to path

SUBDIRS=basic general special tools lock
DESTDIR=/no/such/path
COPYFILES=runtests tests.init server domount.c README READWIN.txt Testitems \
	getopt.c tests.h unixdos.h

include tests.init
CFLAGS := $(shell echo $(CFLAGS))

all: domount getopt
	@set -e; for i in $(SUBDIRS) ; do $(MAKE) -C $$i all ; done
	@chmod a+x runtests

install:

lint:
	@set -e; for i in $(SUBDIRS) ; do $(MAKE) -C $$i lint ; done

domount: domount.c
	$(CC) $(CFLAGS) -o $@ $@.c
	-chown root domount && chmod u+s domount

getopt: getopt.c
	$(CC) $(CFLAGS) -o $@ $@.c

clean:
	rm -f domount getopt
	@set -e; for i in $(SUBDIRS) ; do $(MAKE) -C $$i clean ; done

copy: mknewdirs
	cp -f domount $(COPYFILES) $(DESTDIR)
	@set -e; for i in $(SUBDIRS) ; do \
		$(MAKE) -C $$i copy DESTDIR=$(DESTDIR)/$$i ; \
	done

dist: mknewdirs
	cp -f Makefile $(COPYFILES) $(DESTDIR)
	@set -e; for i in $(SUBDIRS) ; do \
		$(MAKE) -C $$i copy DESTDIR=$(DESTDIR)/$$i ; \
	done
	chmod u+w $(DESTDIR)/tests.init
	chmod a+x $(DESTDIR)/server

mknewdirs:
	-mkdir -p $(DESTDIR)/basic $(DESTDIR)/general $(DESTDIR)/special \
	       $(DESTDIR)/tools $(DESTDIR)/lock
