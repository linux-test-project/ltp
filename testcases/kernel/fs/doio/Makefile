
CFLAGS+= -Wall -I../../../../include
LDLIBS+= -L../../../../lib -lltp

TARGETS=doio growfiles rwtest iogen
SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))

all: $(TARGETS)

doio: doio.o
		 $(CC) -o $@ $^ -I. $(CFLAGS) $(LDLIBS)

growfiles: growfiles.o
		 $(CC) -o $@ $^ $(CFLAGS) $(LDLIBS)

iogen: iogen.o
		 $(CC) -o $@ $^ $(CFLAGS) $(LDLIBS)

rwtest: rwtest.sh doio iogen
	cp rwtest.sh rwtest
	chmod a+rx rwtest

install:
	@set -e; for i in $(TARGETS) ; do ln -f $$i ../../../bin/$$i ; done ;

clean:
	rm -f $(OBJS) $(TARGETS)
