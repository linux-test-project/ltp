#  Makefile for dnssec test

TARGETS=`find . -maxdepth 1 -perm -111 -type f ` 
TARGETS_DIR=`find . -maxdepth 1 -type d` 
TOPDIR=bind9test
TESTDIR=views



all: 

install:      
	mkdir -p ../../../../bin/$(TOPDIR)/$(TESTDIR)
	for i in $(TARGETS) ; do cp -f $$i ../../../../bin/$(TOPDIR)/$(TESTDIR)/$$i || break ; done
	cp -f controls.conf rndc.conf  ../../../../bin/$(TOPDIR)/$(TESTDIR)/
	for i in $(TARGETS_DIR) ; do cp -rpf $$i ../../../../bin/$(TOPDIR)/$(TESTDIR)/ || break ; done

uninstall:
	for f in $(TARGETS) ; do rm -rf ../../../../bin/$(TOPDIR)/$(TESTDIR)/$$f || break ; done

clean distclean:
	~
