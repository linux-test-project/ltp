CFLAGS += -w -O
LDLIBS += 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install:
	ln -f mc_opts ../../../bin
	ln -f opts ../../../bin

clean:
	rm -f $(TARGETS)


