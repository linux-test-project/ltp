TARGETS=selinux_exectrace_parent selinux_exectrace_child
LDLIBS += -lselinux

all: $(TARGETS)

install: 
	@set -e; for i in $(TARGETS); do ln -f $$i ../../../../../bin/$$i; done
	ln -f selinux_exectrace.sh ../../../../../bin/

clean:
	rm -f $(TARGETS)

