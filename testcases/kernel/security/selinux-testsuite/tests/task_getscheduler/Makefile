TARGETS=$(patsubst %.c,%,$(wildcard *.c))

all: $(TARGETS)

install:
	@set -e; for i in $(TARGETS); do ln -f $$i ../../../../../bin/$$i; done
	ln -f selinux_task_getscheduler.sh ../../../../../bin/

clean:
	rm -f $(TARGETS)

