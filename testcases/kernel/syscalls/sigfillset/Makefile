CFLAGS := -Wall -I ../../../../include -D _GNU_SOURCE
LDFLAGS := -L ../../../../lib -lltp 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))
TEST=sigfillset01

all: $(TARGETS)
	@chmod 755 $(TEST)

install:
	for i in $(TARGETS) $(TEST) ; do ln -f $$i ../../../bin/$$i ; done ;

clean:
	rm -f $(TARGETS)
