SUBDIRS =  netpipe-ipv6

all:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

install: all

clean:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done

