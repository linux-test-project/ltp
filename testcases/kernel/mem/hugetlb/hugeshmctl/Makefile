CFLAGS += -I../lib -I../../../../../include -g -Wall
LOADLIBES += -L../../../../../lib -lltp -L../lib -lipc

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)
 
install:
	@set -e; for i in $(TARGETS) ; do ln -f $$i ../../../../bin/$$i ; done ;

clean:
	rm -f $(TARGETS)
