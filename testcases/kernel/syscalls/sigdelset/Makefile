CFLAGS := -Wall -I ../../../../include -D _GNU_SOURCE
LDFLAGS := -L ../../../../lib -lltp 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))
TEST=sigdelset01 sigdelset02 sigdelset03

all: $(TARGETS)

install:
	set -e; for i in $(TARGETS) $(TEST) ; do ln -f $$i ../../../bin/$$i ; done ;
	@chmod 755 $(TEST)

clean:
	rm -f $(TARGETS)
