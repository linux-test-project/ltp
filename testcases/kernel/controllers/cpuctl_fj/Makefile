CFLAGS += -Wall -O2 -Wextra
LOADLIBES+= -lm
CPPFLAGS += -I../../../../include -I../libcontrollers
LDLIBS += -L../../../../lib/ -lltp

SRCS=$(wildcard *.c)
OBJECTS=$(patsubst %.c,%.o,$(SRCS))
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

$(TARGETS): %: %.o

test:
	@./runtest.sh

clean:
	rm -f $(TARGETS) $(OBJECTS)

install:
	@set -e; for i in $(TARGETS) *.sh; do ln -f $$i ../../../bin/$$i ; chmod +x $$i ; done
