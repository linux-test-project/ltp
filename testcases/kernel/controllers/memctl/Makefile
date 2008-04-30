CFLAGS += -Wall
CPPFLAGS += -I../../../../include -I../libcontrollers
LDLIBS += -lm -L../../../../lib/ -L../libcontrollers -lcontrollers -lltp

SRCS    = $(wildcard *.c)

TARGETS = $(patsubst %.c,%,$(SRCS))

all:	$(TARGETS)

clean:
	rm -f $(TARGETS) *.o

install:
	@set -e; for i in $(TARGETS) run_memctl_test.sh myfunctions.sh; do ln -f $$i ../../../bin/$$i ; chmod +x $$i ; done

