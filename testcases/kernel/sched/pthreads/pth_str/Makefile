CFLAGS := -Wall
LDFLAGS := -lpthread

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install:
	@for i in $(TARGETS) ; do ln -f $$i ../../../../bin/$$i ; done ;

clean:
	rm -f $(TARGETS)
