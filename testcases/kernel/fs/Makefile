SUBDIRS = doio fs_inod fsstress fs_perms fsx-linux ftest inode lftest linktest openfile proc stream

all:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

install:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done

clean:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done


