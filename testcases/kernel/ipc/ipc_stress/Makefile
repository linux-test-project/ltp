CFLAGS := -Wall
LDFLAGS := -lpthread

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all:
	make -f Makefile.linux

linux: 
	make -f Makefile.linux

notlinux: $(TARGETS)

clean:
	rm -f $(TARGETS)
