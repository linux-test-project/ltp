CFLAGS := -O -w -g
LDFLAGS := -lpthread

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

clean:
	rm -f $(TARGETS)
