CFLAGS := -Wall -I ../../../../include -D _GNU_SOURCE
LDFLAGS := -L ../../../../lib -lltp 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))
TEST=sigaddset01 sigaddset02 sigaddset03

all: $(TARGETS)

install:
	set -e; for i in $(TARGETS) $(TEST) ; do ln -f $$i ../../../bin/$$i ; done ;
	@chmod 755 $(TEST)

clean:
	rm -f $(TARGETS)
