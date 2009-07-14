SRCS = $(wildcard *.c)

OBJECTS = $(SRCS:.c=.o)

LIBSRCS=$(wildcard ../cpuset_lib/*.c)
LIBOBJECTS=$(patsubst %.c,%.o,$(LIBSRCS))

all: cpuset_syscall_test

cpuset_syscall_test: %: %.o $(LIBOBJECTS)

clean:
	rm -rf cpuset_syscall_test $(OBJECTS) $(LIBOBJECTS)

install:
	@set -e; \
	for i in *.sh; do ln -f $$i ../../../../bin/$$i; chmod +x $$i; done
	ln -f cpuset_syscall_test ../../../../bin/cpuset_syscall_test;
