CFLAGS += -w -O
LDFLAGS += 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install:
	cp nfs01 ../../bin
	mkdir tmp

clean:
	rm -f $(TARGETS)


