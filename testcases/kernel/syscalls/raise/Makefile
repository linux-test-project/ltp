CFLAGS := -Wall -I ../../../../include 
LDFLAGS := -L ../../../../lib -lltp 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))
TEST=raise01

all: $(TARGETS)
	@chmod 755 $(TEST)

install:
	for i in $(TARGETS) $(TEST) ; do ln -f $$i ../../../bin/$$i ; done ;

clean:
	rm -f $(TARGETS)
