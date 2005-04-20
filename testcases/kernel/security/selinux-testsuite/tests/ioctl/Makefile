TARGETS=selinux_ioctl selinux_noioctl

all: $(TARGETS)

install: 
	@set -e; for i in $(TARGETS); do ln -f $$i ../../../../../bin/$$i; done
	ln -f selinux_ioctl.sh ../../../../../bin/

clean:
	rm -f $(TARGETS)
