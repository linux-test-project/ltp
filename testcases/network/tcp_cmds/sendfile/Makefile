CFLAGS += -w -O
LDLIBS += 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

generate:
	./generate.sh

install: generate
	ln -f sendfile01 ../../../bin

clean:
	rm -f $(TARGETS)
	rm -rf datafiles

