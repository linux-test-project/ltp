CFLAGS += -w -O
LDLIBS += 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

generate:
	../../generate.sh

install: generate
	ln -f sendfile601 ../../../bin

clean:
	rm -f $(TARGETS)
	rm -rf datafiles

