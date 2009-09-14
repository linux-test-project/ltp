CFLAGS += -Wall
CPPFLAGS += -I../../../../../include -I../../libcontrollers
LDLIBS += -L../../../../../lib/ -lltp

SRCS    = $(wildcard *.c)

TARGETS = $(patsubst %.c,%,$(SRCS))

all:    $(TARGETS)

clean:
	rm -f $(TARGETS) *.o

install:
	@set -e; for i in $(TARGETS) *.sh; do ln -f $$i ../../../../bin/$$i ; chmod +x $$i ; done

