SUBDIRS = mc_cmds mc_commo mc_member mc_opts

all:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

install:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done


clean:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done
