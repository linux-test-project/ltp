#SUBDIRS = `ls */Makefile | sed "s/Makefile//g"`
SUBDIRS = `ls */Makefile | sed "s/Makefile//g" | grep -v lib6` # Exclude this directory since it breaks on earlier distro versions

all:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

install:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done

clean:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done

