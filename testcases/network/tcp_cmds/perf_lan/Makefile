CFLAGS+=    -I../../../../include -Wall -w -O
LOADLIBES+= -L../../../../lib -lltp
LDLIBS += 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install:
	ln -f perf_lan ../../../bin
	ln -f pingpong ../../../bin

clean:
	rm -f $(TARGETS)


