SUBDIRS =  netpipe-ipv6

all:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

install: all

clean:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done

