CFLAGS += -Wall -g -DNO_XFS -I. -D_LARGEFILE64_SOURCE
LDFLAGS += 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install: all
	ln -s fsstress ../../../bin

clean:
	rm -f $(TARGETS)


