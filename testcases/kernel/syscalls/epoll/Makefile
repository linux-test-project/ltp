CFLAGS+= -Wall -O -g -I../../../../include

LIBC_VER=$(shell ldd --version | grep libc |  cut -d " " -f4)
CUR_LIBC_VER:=$(LIBC_VER)
EPOLL_INC:=2.3.2
VER_LEVEL=$(shell ./CheckVer $(EPOLL_INC) $(CUR_LIBC_VER) | grep Return | cut -d " " -f2)
BASE_VER=1

ifneq (${BASE_VER},${VER_LEVEL})
LDLIBS+= -L /lib -L../../../../lib -lepoll -lltp
else
LDLIBS+= -lltp
endif



TARGETS=CheckVer epoll-ltp
SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))

all: $(TARGETS)

epoll-ltp: epoll-ltp.o
		$(CC) -o $@ $^  $(CFLAGS) $(LDLIBS)

CheckVer: CheckVer.o
		$(CC) -o $@ $^ $(CFLAGS)

install:
	@set -e; for i in $(TARGETS); do ln -f $$i ../../../bin/$$i ; done

clean:
	rm -f $(OBJS) $(TARGETS)

