CFLAGS += -c -O
LDFLAGS += 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%.o,$(SRCS))

all: $(TARGETS)

install:
	cp ldd01 ../../bin

clean:
	rm -f $(TARGETS)


