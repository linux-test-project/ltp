SUBDIRS = `ls */Makefile | grep -v open_posix_testsuite | sed "s/Makefile//g"`

all:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

posix:
	@set -e; $(MAKE) -C open_posix_testsuite

install:
	mkdir -p bin
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done

clean:
	rm -rf bin/
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done

