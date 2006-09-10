SUBDIRS = include fs io ipc mem pty sched security syscalls
UCLINUX_SUBDIRS = syscalls

all:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

install:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done

uclinux:
	@set -e; for i in $(UCLINUX_SUBDIRS); do $(MAKE) -C $$i uclinux ; done

uclinux_install:
	@set -e; for i in $(UCLINUX_SUBDIRS); do $(MAKE) -C $$i uclinux_install ; done

clean:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done
