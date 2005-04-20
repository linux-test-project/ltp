TARGETS=selinux_wait_parent selinux_wait_child
LDLIBS += -lselinux

all: $(TARGETS)
install:
	@set -e; for i in $(TARGETS); do ln -f $$i ../../../../../bin/$$i; done
	ln -f selinux_wait.sh ../../../../../bin/

clean:
	rm -f $(TARGETS)
