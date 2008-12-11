CFLAGS+= -Wall -O -g -DAIO -I../../../../include
LDLIBS+= -L../../../../lib -laio -lltp

TARGETS=ltp-diorh aiocp aiodio_append aiodio_sparse dio_append dio_sparse dio_truncate dirty read_checkzero aio-stress
SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))

all: $(TARGETS)

ltp-diorh: ltp-diorh.o
		 $(CC) -o $@ $^ -I. $(CFLAGS) $(LDLIBS)

aiocp: aiocp.o
		 $(CC) -o $@ $^ -I. $(CFLAGS) $(LDLIBS)

aiodio_append: aiodio_append.o
		 $(CC) -o $@ $^ $(CFLAGS) $(LDLIBS)

aiodio_sparse: aiodio_sparse.o
		 $(CC) -o $@ $^ $(CFLAGS) $(LDLIBS)

dio_append: dio_append.o
		 $(CC) -o $@ $^ $(CFLAGS) $(LDLIBS)

dio_sparse: dio_sparse.o
		 $(CC) -o $@ $^ $(CFLAGS) $(LDLIBS)

dio_truncate: dio_truncate.o
		 $(CC) -o $@ $^ $(CFLAGS) $(LDLIBS)

dirty: dirty.o
		 $(CC) -o $@ $^ $(CFLAGS) $(LDLIBS)

read_checkzero: read_checkzero.o
		 $(CC) -o $@ $^ $(CFLAGS) $(LDLIBS)

aio-stress: aio-stress.o
		 $(CC) -o $@ $^ $(CFLAGS) $(LDLIBS) -lpthread

install:
	@set -e; for i in $(TARGETS) ; do ln -f $$i ../../../bin/$$i ; done ;

clean:
	rm -f $(OBJS) $(TARGETS)
