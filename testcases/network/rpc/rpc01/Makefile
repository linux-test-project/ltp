CFLAGS += -w -O
LDLIBS += 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install:
	ln -f rpc01 ../../../bin
	ln -f rpc1 ../../../bin
	ln -f rpc_server ../../../bin

clean:
	rm -f $(TARGETS)


