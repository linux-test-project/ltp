SUBDIRS = ade cli_sev files fs io mem multicast nfs pth_str rpc tcp_cmds

all:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

install:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done

clean:
	rm -f bin/*
	@for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done

