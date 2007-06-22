SUBDIRS = libclone utsname sysvipc

all noltp noltp_check:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i $@; done

install:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done
	chmod ugo+x container_test.sh
	ln -f container_test.sh ../../bin/container_test.sh

clean:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done


