
DESTDIR=/opt/ltp

CFLAGS+= -Wall
CPPFLAGS+= -I../include 
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

install: $(TARGET)
	install -D $(TARGET) $(DESTDIR)/lib/$(TARGET)
	install -D ltp.pc /usr/share/pkgconfig/ltp.pc

