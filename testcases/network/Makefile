SUBDIRS = multicast nfs rpc tcp_cmds ipv6 

all:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

install:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done

clean:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done

