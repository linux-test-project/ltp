#  Makefile for bind9 test

TARGETS=`find . -maxdepth 1 -perm -111 -type f`
TESTDIR=bind9test

all: 

install:        all
	mkdir -p ../../../bin/$(TESTDIR)
	for i in $(TARGETS) ; do ln -f $$i ../../../bin/$(TESTDIR)/$$i || break ; done
	$(MAKE) -C forward install
	$(MAKE) -C xfer   install
	$(MAKE) -C upforwd install
	$(MAKE) -C views  install
        

uninstall:
	for f in $(TARGETS) ; do rm -f ../../../bin/$(TESTDIR)/$$f || break ; done
	rm -rf ../../../bin/$(TESTDIR)


clean distclean:
