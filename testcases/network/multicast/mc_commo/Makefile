CFLAGS += -O -Wall
LDLIBS += 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install:
	ln -f mc_commo ../../../bin

clean:
	rm -f $(TARGETS)


