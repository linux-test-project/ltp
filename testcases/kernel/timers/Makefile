SUBDIRS = `ls */Makefile | sed "s/Makefile//g"`

all:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i ; ln -fs ../chmod.sh $$i/chmod.sh; done

install:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done

clean:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; rm -f $$i/chmod.sh; done


