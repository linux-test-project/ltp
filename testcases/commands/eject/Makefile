CFLAGS+=    -I../../../include
LOADLIBES+= -L../../../lib -lltp

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install:
	@for i in $(TARGETS); do ln -f $$i ../../bin/$$i ; done
	@ln -f eject-tests.sh ../../bin/eject_tests.sh
