
CFLAGS+= -Wall -I../include
LDFLAGS+= -L../lib -lltp

TARGETS=doio growfiles rwtest iogen
SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))

all: $(TARGETS)

doio: doio.o
	$(CC) -o $@ $^ -I. $(CFLAGS) $(LDFLAGS)

growfiles: growfiles.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

iogen: iogen.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

rwtest: rwtest.sh doio iogen
	cp rwtest.sh rwtest
	chmod a+rx rwtest

clean:
	rm -f $(OBJS) $(TARGETS)
