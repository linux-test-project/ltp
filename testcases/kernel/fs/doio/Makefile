
CFLAGS+= -Wall -I../../../../include -D_LARGEFILE64_SOURCE
LDLIBS+= -L../../../../lib -lltp -lrt

TARGETS=doio growfiles rwtest iogen
SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))

all: $(TARGETS)

doio: doio.o
		 $(CC) -o $@ $^ -I. $(CFLAGS) $(LDFLAGS) $(LDLIBS)

growfiles: growfiles.o
		 $(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LDLIBS)

iogen: iogen.o
		 $(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LDLIBS)

rwtest: rwtest.sh doio iogen
	cp rwtest.sh rwtest
	chmod a+rx rwtest

install:
	@set -e; for i in $(TARGETS) ; do ln -f $$i ../../../bin/$$i ; done ;

clean:
	rm -f $(OBJS) $(TARGETS)
