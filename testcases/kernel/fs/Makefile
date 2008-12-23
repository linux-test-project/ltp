SUBDIRS = acls doio fs_inod fsstress fs_perms fsx-linux ftest inode lftest linktest openfile proc stream fs_di fs_bind racer

all:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

install:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done

clean:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done


