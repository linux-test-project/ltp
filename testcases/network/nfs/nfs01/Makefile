CFLAGS += -w -O
LDLIBS += 

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install:
	ln -f nfs01 ../../../bin; ln -f open_files ../../../bin

clean:
	rm -f $(TARGETS)


