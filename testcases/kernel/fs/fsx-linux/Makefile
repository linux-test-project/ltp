CFLAGS += -w -O
LDFLAGS += 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install: all
	ln -f fsxtest ../../../bin

clean:
	rm -f $(TARGETS)


