CFLAGS += -Wall
LDFLAGS += 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install: all
	ln -s fsstress ../../../bin

clean:
	rm -f $(TARGETS)


