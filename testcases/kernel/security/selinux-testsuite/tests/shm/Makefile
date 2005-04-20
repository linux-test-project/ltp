TARGETS=$(patsubst %.c,%,$(wildcard *.c))

all: $(TARGETS)

install:
	@set -e; for i in $(TARGETS); do ln -f $$i ../../../../../bin/$$i; done
	ln -f selinux_shm.sh ../../../../../bin/

clean:
	rm -f $(TARGETS)
