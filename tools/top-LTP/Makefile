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

CFLAGS += -I/usr/include -w -D_GNU_SOURCE -O2 -g3 -fno-common -ffast-math -I proc \
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

ifeq ($(LIB_DIR),)
LIB_DIR=/lib
endif

install: 
	@set -e; ln -f top ../../testcases/bin
	@set -e; cp -f proc/libproc.so.$(LIBVERSION) $(LIB_DIR) 
	@set -e; ln -fs $(LIB_DIR)/libproc.so.$(LIBVERSION) $(LIB_DIR)/libproc.so
 
############ prog.c --> prog.o

%.o : %.c
	@set -e;$(CC) $(CFLAGS) -c -o $@ $^ 

############ prog.o --> prog

top:   % : %.o $(LIBPROC)
	@set -e;$(CC) $(LDFLAGS) -o $@ $^ $(CURSES) 


