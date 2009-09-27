CFLAGS += -O -w -g
LDLIBS := -lpthread

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install:
	ln -f nfsstress ../../../bin
	ln -f $(TARGETS) ../../../bin

clean:
	rm -f $(TARGETS)
