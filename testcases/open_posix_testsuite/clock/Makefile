CFLAGS += -Wall -I ../../../include -D _GNU_SOURCE
LDLIBS := -L ../../../lib -lltp 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))
TEST=clock01

all: $(TARGETS)

install:
	set -e; for i in $(TARGETS) $(TEST) ; do ln -f $$i ../../bin/$$i ; done ;

clean:
	rm -f $(TARGETS)
