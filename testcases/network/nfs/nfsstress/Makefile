CFLAGS := -O -w -g
LDFLAGS := -lpthread

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install:
	ln -f nfsstress ../../../bin

clean:
	rm -f $(TARGETS)
