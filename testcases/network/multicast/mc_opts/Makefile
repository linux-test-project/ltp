CFLAGS += -Wall -g 
LDLIBS += 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install:
	@set -e;ln -f mc_opts ../../../bin
	@set -e;ln -f opts ../../../bin
	@set -e;ln -f opts_e ../../../bin

clean:
	rm -f $(TARGETS)


