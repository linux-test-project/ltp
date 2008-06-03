# Makefile for linux memory toy/tool
#
SHELL   = /bin/sh

MACH    =

CMODE	= -std=gnu99
COPT	= $(CMODE) -O2 #-non_shared
DEFS    = -D_GNU_SOURCE -D_NEED_MIGRATE_PAGES
INCLS   =  #-I
CFLAGS  = $(COPT) $(DEFS) $(INCLS) $(ECFLAGS)

LDOPTS	= #-dnon_shared
LDLIBS	= -lnuma
LDFLAGS = $(CMODE) $(LDOPTS) $(ELDFLAGS)

HDRS    = memtoy.h segment.h migrate_pages.h

OBJS    = memtoy.o commands.o segment.o migrate_pages.o

PROGS	= memtoy

#---------------------------------

all:    $(PROGS)

memtoy:  $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS) $(LDLIBS)

$(OBJS):    $(HDRS)


install:
	@echo "install not implemented"

clean:
	-rm -f *.o core.[0-9]* Log* cscope.*

clobber: clean
	-rm -f  $(PROGS)

# ------------------------------------------------
TARBALL=memtoy-$$(cat $$_WD/version.h|grep _VERSION|sed 's/^.* "\([0-9.a-z]*\)".*$$/\1/').tar.gz
tarball:  clobber
	_WD=`pwd`; \
	_WD=`basename $$_WD`; \
	cd ..; tar czf - $$_WD  >$(TARBALL)
