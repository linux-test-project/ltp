CFLAGS += -O
LDFLAGS += 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install:
	cp mc_commo ../../bin

clean:
	rm -f $(TARGETS)


