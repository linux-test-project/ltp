CFLAGS += -Wall -I ../../../../../include -I ../include -D _GNU_SOURCE
LDLIBS := -L ../../../../../lib -lpthread -lltp

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))
TEST=`ls | grep 01 | grep -v '\.'`

all: $(TARGETS)

install:
	set -e; for i in $(TARGETS) $(TEST) ; do ln -f $$i ../../../../bin/$$i ; done ;
	@/bin/sh ./chmod.sh $(TEST)

clean:
	rm -f $(TARGETS)
