
CFLAGS+= -Wall -I../include
LOADLIBES+= -L../lib -lltp

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

clean:
	rm -f $(TARGETS)
