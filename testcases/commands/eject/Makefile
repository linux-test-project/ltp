CFLAGS+=    -I../../../include -Wall
LOADLIBES+= -L../../../lib -lltp

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install:
	@set -e; for i in $(TARGETS); do ln -f $$i ../../bin/$$i ; done
	@ln -f eject-tests.sh ../../bin/eject_tests.sh

clean:
	rm -f $(TARGETS)
