CFLAGS += -w -O
LDFLAGS += 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS) object

install:
	cp ar01 ../../bin

object:
	cc -c file1.c

clean:
	rm -f $(TARGETS) file1.o


