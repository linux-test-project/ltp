CFLAGS += -w -O
LDFLAGS += 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install:
	ln -f fsxtest02 ../../../bin
	ln -f fsxtest ../../../bin
	ln -f fsx-linux ../../../bin

clean:
	rm -f $(TARGETS)


