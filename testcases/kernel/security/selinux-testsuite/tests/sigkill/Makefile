TARGETS=selinux_sigkill_server

all: $(TARGETS)

install:
	@set -e; for i in $(TARGETS); do ln -f $$i ../../../../../bin/$$i; done
	ln -f selinux_sigkill.sh ../../../../../bin/

clean:
	rm -f $(TARGETS) 
