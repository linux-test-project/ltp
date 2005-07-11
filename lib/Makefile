
ifeq ($(shell uname -s),SunOS)
CC=gcc -Wall
endif
CFLAGS+= -I../include 
ifeq ($(shell uname -s),Linux)
CFLAGS+= -Wall
endif
ifeq ($(shell uname -s),HP-UX)
CFLAGS+=-Ae -D_LARGEFILE64_SOURCE +DA1.1
endif
CFLAGS+=$(if $(UCLINUX),-DUCLINUX)
LDFLAGS+=
TARGET=libltp.a
SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))

$(TARGET): $(OBJS)
	$(AR) -cr $@ $^    

clean:
	rm -f $(TARGET) $(OBJS)
