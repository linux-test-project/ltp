SUBDIRS = `ls */Makefile | sed "s/Makefile//g"`

all:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

install:
	mkdir -p bin
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done

clean:
	rm -rf bin/
	@set -e; for i in $(SUBDIRS) ; do $(MAKE) -C $$i clean ; done

