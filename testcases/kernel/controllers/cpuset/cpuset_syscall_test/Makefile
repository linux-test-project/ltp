SRCS = $(wildcard *.sh)

all:cpuset_syscall_test

clean:
	rm -rf cpuset_syscall_test

install:
	@set -e;
	for i in $(SRCS); do ln -f $$i ../../../../bin/$$i; chmod +x $$i; done
	ln -f cpuset_syscall_test ../../../../bin/cpuset_syscall_test;
