SUBDIRS = `ls */Makefile | sed "s/Makefile//g"`

# Use this "all" if you want to continue on errors.
#
#all:
#	@set -e; for i in $(SUBDIRS); do $(MAKE) -C -i $$i ; done

all:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

install:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done

clean:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done


