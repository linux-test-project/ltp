CFLAGS := -Wall -I ../../../../include -D _GNU_SOURCE
LDFLAGS := -L ../../../../lib -lltp 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))
TEST=sigismember01 sigismember02 

all: $(TARGETS)

install:
	set -e; for i in $(TARGETS) $(TEST) ; do ln -f $$i ../../../bin/$$i ; done ;
	@for i in $(TEST); do /bin/sh ./chmod.sh $$i; done

clean:
	rm -f $(TARGETS)
