CFLAGS += -w -O 
LDFLAGS += 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install: all
	ln -f echo601 ../../../bin

clean:
	rm -f $(TARGETS)


