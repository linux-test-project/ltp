CFLAGS += -Wall -I ../../../include -I ../include -D_GNU_SOURCE
LDLIBS := -L ../../../lib -lltp -lmqueue

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))
TEST=`ls | grep 01 | grep -v '\.'`

all: $(TARGETS)

install:
	set -e; for i in $(TARGETS) $(TEST) ; do ln -f $$i ../../bin/$$i ; done ;

clean:
	rm -f $(TARGETS)
