CFLAGS += -O -Wall
LDLIBS += 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install:
	ln -f mc_commo ../../../bin
	ln -f mc_recv ../../../bin
	ln -f mc_send ../../../bin

clean:
	rm -f $(TARGETS)


