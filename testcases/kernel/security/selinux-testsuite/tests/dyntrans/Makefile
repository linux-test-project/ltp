TARGETS=$(patsubst %.c,%,$(wildcard *.c))
LDLIBS += -lselinux

all: $(TARGETS)

install:
	@set -e; for i in $(TARGETS); do ln -f $$i ../../../../../bin/$$i; done
	ln -f selinux_dyntrans.sh ../../../../../bin/

clean:
	rm -f $(TARGETS)
