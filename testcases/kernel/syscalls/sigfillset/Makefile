CFLAGS := -Wall -I ../../../../include -D _GNU_SOURCE
LDLIBS := -L ../../../../lib -lltp 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))
TEST=sigfillset01

all: $(TARGETS)

install:
	set -e; for i in $(TARGETS) $(TEST) ; do ln -f $$i ../../../bin/$$i ; done ;
	@/bin/sh ./chmod.sh $(TEST)

clean:
	rm -f $(TARGETS)
