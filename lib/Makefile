
NPTL:=$(shell LD_ASSUME_KERNEL=2.4.1 getconf GNU_LIBPTHREAD_VERSION 2>/dev/null| grep NPTL; echo $?)
ifeq ($(NPTL),)
CFLAGS+= -Wall -I../include 
else
CFLAGS+= -Wall -I../include -DUSE_NPTL
endif

LDFLAGS+=
TARGET=libltp.a
SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))

$(TARGET): $(OBJS)
	$(AR) -cr $@ $^    

clean:
	rm -f $(TARGET) $(OBJS)
