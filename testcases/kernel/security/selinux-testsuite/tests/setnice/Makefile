TARGETS=selinux_setnice_parent selinux_setnice_child

LDLIBS += -lselinux

all: $(TARGETS)
install:
	@set -e; for i in $(TARGETS); do ln -f $$i ../../../../../bin/$$i; done
	ln -f selinux_setnice.sh ../../../../../bin/

clean:
	rm -f $(TARGETS)
