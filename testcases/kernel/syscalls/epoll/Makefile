CFLAGS+= -Wall -O -g -I../../../../include
LDLIBS+= -L /lib -L../../../../lib -lepoll -lltp

TARGETS=epoll-ltp
SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))

all: $(TARGETS)

epoll-ltp: epoll-ltp.o
		 $(CC) -o $@ $^  $(CFLAGS) $(LDLIBS)


clean:
	rm -f $(OBJS) $(TARGETS)
