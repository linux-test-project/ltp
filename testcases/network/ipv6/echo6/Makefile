CFLAGS += -w -O 
LDLIBS += 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install:
	ln -f echo601 ../../../bin

clean:
	rm -f $(TARGETS)


