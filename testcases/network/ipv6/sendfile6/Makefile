CFLAGS += -w -O
LDFLAGS += 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS) generate

generate:
	./generate.sh

install: 
	ln -f sendfile601 ../../../bin

clean:
	rm -f $(TARGETS)
	rm -rf datafiles

