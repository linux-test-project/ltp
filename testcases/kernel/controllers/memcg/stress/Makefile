CFLAGS += -Wall -O2 -g -Wextra
LDLIBS += -lm

SRCS=$(wildcard *.c)
OBJECTS=$(patsubst %.c,%.o,$(SRCS))
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

$(TARGETS): %: %.o

install:
	@set -e; for i in $(TARGETS) *.sh; do ln -f $$i ../../../../bin/$$i ; chmod +x $$i ; done

test:
	@./memcgroup_stress_test.sh

clean:
	rm -f $(TARGETS) $(OBJECTS)

uninstall:

.DEFAULT: ;
