CFLAGS += -Wall -g -Wextra

LIBSRCS=$(wildcard ../cpuset_lib/*.c)
LIBOBJECTS=$(patsubst %.c,%.o,$(LIBSRCS))

all: cpuset_memory_test

cpuset_memory_test: cpuset_memory_test.o $(LIBOBJECTS)
	cc cpuset_memory_test.o $(LIBOBJECTS) -o cpuset_memory_test -lpthread

clean:
	rm -f cpuset_memory_test cpuset_memory_test.o
	rm -f $(LIBOBJECTS)

install:
	@set -e;
	ln -f cpuset_memory_test ../../../../bin/cpuset_memory_test;
	ln -f cpuset_memory_testset.sh ../../../../bin/cpuset_memory_testset.sh;
	chmod +x cpuset_memory_test;
	chmod +x cpuset_memory_testset.sh;
