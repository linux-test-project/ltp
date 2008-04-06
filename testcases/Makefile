ifeq ($(shell uname -m |grep -c -e "86" -e "ppc"), 1) 
	SUBDIRS = `ls */Makefile | sed "s/Makefile//g" | grep -v open | grep -v pounder | grep -v DOTS | grep -v kdump ` 
else	
	SUBDIRS = `ls */Makefile | sed "s/Makefile//g" | grep -v open | grep -v pounder | grep -v DOTS | grep -v kdump | grep -v realtime` 
endif

UCLINUX_SUBDIRS = kernel
all:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

install:
	mkdir -p bin
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done

uclinux:
	@set -e; for i in $(UCLINUX_SUBDIRS); do $(MAKE) -C $$i uclinux; done

uclinux_install:
	mkdir -p bin
	@set -e; for i in $(UCLINUX_SUBDIRS); \
		do $(MAKE) -C $$i uclinux_install ; done

clean:
	rm -rf bin/
	@set -e; for i in $(SUBDIRS) ; do $(MAKE) -C $$i clean ; done

