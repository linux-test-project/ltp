CFLAGS += -w -O 
LDLIBS += 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install:
	ln -f echo01 ../../../bin

clean:
	rm -f $(TARGETS)


