CFLAGS += -w -O
LDLIBS += 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install:
	ln -f mc_member ../../../bin

clean:
	rm -f $(TARGETS)


