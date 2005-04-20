TARGETS=selinux_task_create_parent

all: $(TARGETS)
install:
	@set -e; for i in $(TARGETS); do ln -f $$i ../../../../../bin/$$i; done
	ln -f selinux_task_create.sh ../../../../../bin/

clean:
	rm -f $(TARGETS)
