CFLAGS := -Wall -I ../../../../../include -I ../include -D _GNU_SOURCE
LDFLAGS := -L ../../../../../lib -lpthread -lltp

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))
TEST=`ls | grep 01 | grep -v '\.'`

all: $(TARGETS)

install:
	set -e; for i in $(TARGETS) $(TEST) ; do ln -f $$i ../../../../bin/$$i ; done ;
	@chmod 755 $(TEST)

clean:
	rm -f $(TARGETS)
