CFLAGS += -Wall -O -g
LDLIBS := -lpthread

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install:
	ln -f $(TARGETS) ../../../bin

clean:
	rm -f $(TARGETS)

