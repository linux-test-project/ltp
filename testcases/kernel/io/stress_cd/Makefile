CFLAGS := -O -w -g
LDFLAGS := -lpthread

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install: all
	ln -f $(TARGETS) ../../../bin

clean:
	rm -f $(TARGETS)

