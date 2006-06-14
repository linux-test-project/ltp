CFLAGS += -Wall -I ../../../../include -I ../include
LDLIBS := -L ../../../../lib -lpthread -lltp

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)


install:
	set -e; for i in $(TARGETS) ; do ln -f $$i ../../../bin/$$i ; done ;

clean:
	rm -f $(TARGETS)
