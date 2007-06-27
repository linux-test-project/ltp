SUBDIRS = libclone utsname sysvipc

all:
	@set -e; $(MAKE) check_for_unshare; \
	if './check_for_unshare' > /dev/null 2>&1; then \
		for i in $(SUBDIRS); do $(MAKE) -C $$i $@; done \
	else echo "system does not support unshare"; true; fi

noltp noltp_check:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i $@; done

check_for_unshare: check_for_unshare.c
	$(CC) -o $@ $< ../../../lib/tst_kvercmp.c -I../../../include

install:
	@set -e; ln -f check_for_unshare ../../bin/check_for_unshare; \
	ln -f container_test.sh ../../bin/container_test.sh; \
	if './check_for_unshare' > /dev/null 2>&1; then \
		for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done; \
		chmod ugo+x container_test.sh; \
	else echo "system does not support unshare"; true; fi

clean:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done
	rm -f check_for_unshare
