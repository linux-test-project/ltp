CFLAGS += -w -O
LDFLAGS += 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install:
	cp mc_member ../../bin

clean:
	rm -f $(TARGETS)


