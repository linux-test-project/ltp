VERSION      := 3
SUBVERSION   := 1
MINORVERSION := 5
TARVERSION   := 3.1.5
LIBVERSION   := 3.1.5

ldconfig := ldconfig
ln_f     := ln -f
ln_sf    := ln -sf


CURSES := -lncurses

LDFLAGS += -Wl,-warn-common

CFLAGS += -w -D_GNU_SOURCE -O2 -g3 -fno-common -ffast-math -I proc \
  -W -Wall -Wshadow -Wcast-align -Wredundant-decls \
  -Wbad-function-cast -Wcast-qual -Wwrite-strings -Waggregate-return \
#  -Wpadded -Wunreachable-code -Wdisabled-optimization \
  -Wstrict-prototypes -Wmissing-prototypes # -Wconversion


all: do_all

-include */module.mk

do_all: top

clean:
	rm -f top top.o proc/libproc.* proc/*.o

###### install

install: 
	@set -e; ln -f top ../../testcases/bin
	@set -e; cp -f proc/libproc.* /lib
 
############ prog.c --> prog.o

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $^

############ prog.o --> prog

top:   % : %.o $(LIBPROC)
	$(CC) $(LDFLAGS) -o $@ $^ $(CURSES)


