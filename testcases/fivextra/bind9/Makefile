#  Makefile for bind9 test

TESTDIR=bind9test

all:
	$(MAKE) -C bind9test

install:        
	ln -f ./bind9.sh ../../bin
	$(MAKE) -C bind9test install

uninstall:
	rm -f ../../bin/bind9.sh
	rm -rf ../../bin/$(TESTDIR)

clean distclean:
	$(MAKE) -C bind9test clean
