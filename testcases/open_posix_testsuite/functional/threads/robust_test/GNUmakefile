include ../Makefile.inc

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))
CFLAGS+=-Wall -g -I../include

all:$(TARGETS)

clean:
	rm -rf $(TARGETS)
