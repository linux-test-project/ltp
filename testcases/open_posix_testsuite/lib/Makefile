top_srcdir?=		..
subdir=			lib

AR?=			ar
RANLIB?=		ranlib
CFLAGS+=		-I$(top_srcdir)/include


vpath %.c $(top_srcdir)/$(subdir)

all: libcommon.a

clean:
	rm -f libcommon.a *.o

libcommon.a: common.o
	$(AR) -rc "$@" $^
	$(RANLIB) "$@"
