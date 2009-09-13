CFLAGS += -w -O
LDLIBS += 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install:
	ln -f mc_member ../../../bin
	ln -f member ../../../bin
	ln -f ManyGroups ../../../bin
	ln -f TooManyGroups ../../../bin

clean:
	rm -f $(TARGETS)


