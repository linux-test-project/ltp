SUBDIRS = fsx-linux nfslock01 nfs01 nfs02 nfs03 nfsstat01 nfsstress

all:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

install:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done


clean:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done
