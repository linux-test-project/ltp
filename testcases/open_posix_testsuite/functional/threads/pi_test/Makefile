include ../Makefile.inc

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))
CFLAGS+=-Wall -D_GNU_SOURCE -g -I../include

all:$(TARGETS)

clean:
	rm -rf $(TARGETS)
