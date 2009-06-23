CFLAGS += -Wall -g -Wextra
LDLIBS := -lm

LIBSRCS=$(wildcard ../cpuset_lib/*.c)
LIBOBJECTS=$(patsubst %.c,%.o,$(LIBSRCS))

SRCS=$(wildcard *.c)
OBJECTS=$(patsubst %.c,%.o,$(SRCS))
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

$(TARGETS): %: %.o $(LIBOBJECTS)

clean:
	rm -f $(TARGETS) $(OBJECTS) $(LIBOBJECTS)

install:
	@set -e; \
	for i in $(TARGETS); do ln -f $$i ../../../../bin/$$i; chmod +x $$i; done
	ln -f cpuset_load_balance_test.sh ../../../../bin/cpuset_load_balance_test.sh;
	ln -f cpuset_sched_domains_test.sh ../../../../bin/cpuset_sched_domains_test.sh;
	chmod +x cpuset_load_balance_test.sh;
	chmod +x cpuset_sched_domains_test.sh;
