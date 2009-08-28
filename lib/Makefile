
PREFIX=/opt/ltp

CFLAGS+= -Wall
CPPFLAGS+= -I../include 
ifeq ($(shell uname -s),HP-UX)
CFLAGS+=-Ae -D_LARGEFILE64_SOURCE +DA1.1
endif
ifeq ($(UCLINUX),1)
CFLAGS+= -D_USC_LIB_ -D__UCLIBC__ -DUCLINUX
endif
LDFLAGS+=
TARGET=libltp.a
SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(AR) -cr $@ $^

clean:
	rm -f $(TARGET) *.o

install: $(TARGET)
	install -D -m 644 $(TARGET) $(DESTDIR)/$(PREFIX)/lib$(LIBSUFFIX)/$(TARGET)
	install -D -m 644 ltp.pc $(DESTDIR)/$(PREFIX)/share/pkgconfig/ltp.pc

