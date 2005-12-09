##############################################################
# Note: The acl directory is intentionally excluded since it
# requires packages that are not usually installed by default
# Install the attr and acl packages to build the acl testsuite
##############################################################


SUBDIRS = `ls */Makefile | sed "s/Makefile//g" | grep -v acl`

all:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

install:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done

clean:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done

