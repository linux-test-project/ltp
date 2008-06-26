SUBDIRS = numa containers controllers include fs io ipc mem pty sched security syscalls timers
# Will be adding conectors here in future
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
	@set -e; for i in $(SUBDIRS) timers; do $(MAKE) -C $$i clean ; done
