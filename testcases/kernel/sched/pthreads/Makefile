CFLAGS := -Wall
LDFLAGS := -lpthread
CC = gcc

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install: all
	@for i in $(TARGETS) ; do ln -f $$i ../../../bin/$$i ; done ;

clean:
	rm -f $(TARGETS)
