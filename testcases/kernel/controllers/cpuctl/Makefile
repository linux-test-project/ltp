CFLAGS += -Wall
CPPFLAGS += -I../../../../include
LDLIBS += -lm -L../../../../lib/ -lltp

SRCS    = $(wildcard *.c)

TARGETS = $(patsubst %.c,%,$(SRCS))

all:	$(TARGETS)

clean:
	rm -f $(TARGETS) *.o

install:
	@set -e; for i in $(TARGETS) run_cpuctl_test.sh run_cpuctl_stress_test.sh parameters.sh ; do ln -f $$i ../../../bin/$$i ; chmod +x $$i ; done

