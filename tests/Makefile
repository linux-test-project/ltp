
CFLAGS+= -I../include
LOADLIBES+= -L../lib -lltp

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

#$(TARGETS): $(SRCS)
#	$(CC) -o $@ $@.c $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(TARGETS)
