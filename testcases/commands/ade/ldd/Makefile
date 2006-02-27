CFLAGS += -c -O -fpic
LDFLAGS += 

SRCS:=$(wildcard *.c)
TARGETS:=$(patsubst %.c,%.o,$(SRCS))

all: $(TARGETS)

install:
	ln -f ldd01 ../../../bin
	ln -f *.o ../../../bin

clean:
	rm -f $(TARGETS)
