#CFLAGS := -Wall
LDFLAGS := -lpthread

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install: all
	cp -a $(TARGETS) ../bin

clean:
	rm -f $(TARGETS)
