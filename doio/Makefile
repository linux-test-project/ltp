
CFLAGS+=
LDFLAGS+=
TARGETS=doio iogen rwtest growfiles

SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))

all: $(TARGETS)

doio: doio.o random_range.o string_to_tokens.o pattern.o write_log.o
	$(CC) -o $@ $^ $(LDFLAGS)

growfiles: growfiles.o dataascii.o open_flags.o tlibio.o random_range.o \
           file_lock.o datapid.o databin.o forker.o string_to_tokens.o 
	$(CC) -o $@ $^ $(LDFLAGS)

iogen: iogen.o str_to_bytes.o string_to_tokens.o random_range.o \
       open_flags.o
	$(CC) -o $@ $^ $(LDFLAGS)

rwtest: rwtest.ks doio iogen
	cp rwtest.ks rwtest
	chmod a+rx rwtest

clean:
	rm -f $(OBJS) $(TARGETS)
