CFLAGS += -Wall -g -DNO_XFS -I. -D_LARGEFILE64_SOURCE -D_GNU_SOURCE
LDFLAGS += 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install:
	ln -f fsstress ../../../bin

clean:
	rm -f $(TARGETS)


