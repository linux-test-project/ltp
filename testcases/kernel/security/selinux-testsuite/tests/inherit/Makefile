TARGETS=selinux_inherit_child selinux_inherit_parent

LDFLAGS +=-static

LDLIBS += -lselinux

all: $(TARGETS)

install: 
	@set -e; for i in $(TARGETS); do ln -f $$i ../../../../../bin/$$i; done
	ln -f selinux_inherit.sh ../../../../../bin/

clean:
	rm -f $(TARGETS)
