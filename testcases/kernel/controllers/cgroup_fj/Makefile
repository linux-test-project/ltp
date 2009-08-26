CFLAGS += -Wall -O2 -Wextra

SRCS    = $(wildcard *.c)
TARGETS = $(patsubst %.c,%,$(SRCS))

all:	$(TARGETS)

install:

test:
	@./run_cgroup_test_fj.sh

clean:
	rm -f $(TARGETS) *.o

install:
	@set -e; for i in $(TARGETS) *.sh; do ln -f $$i ../../../bin/$$i ; chmod +x $$i ; done
	ln -f cgroup_fj_release_agent ../../../bin/cgroup_fj_release_agent
	ln -f cgroup_fj_testcases ../../../bin/cgroup_fj_testcases 
