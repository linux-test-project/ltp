# procps Makefile
# Albert Cahalan, 2002
#
# Recursive make is considered harmful:
# http://google.com/search?q=%22recursive+make+considered+harmful%22
#
# For now this Makefile uses explicit dependencies. The project
# hasn't grown big enough to need something complicated, and the
# dependency tracking files are an ugly annoyance.
#
# This file includes */module.mk files which add on to variables:
# FOO += bar/baz
#
#
# Set (or uncomment) SKIP if you wish to avoid something.
# For example, you may prefer the /bin/kill from util-linux or bsdutils.


VERSION      := 3
SUBVERSION   := 1
MINORVERSION := 5
TARVERSION   := 3.1.5
LIBVERSION   := 3.1.5

############ vars

# so you can disable them or choose alternates
ldconfig := ldconfig
ln_f     := ln -f
ln_sf    := ln -sf
install  := install -D --owner 0 --group 0

usr/bin                  := $(DESTDIR)/usr/bin/
bin                      := $(DESTDIR)/bin/
sbin                     := $(DESTDIR)/sbin/
usr/proc/bin             := $(DESTDIR)/usr/bin/
man1                     := $(DESTDIR)/usr/share/man/man1/
man5                     := $(DESTDIR)/usr/share/man/man5/
man8                     := $(DESTDIR)/usr/share/man/man8/
etc/X11/applnk/Utilities := $(DESTDIR)/etc/X11/applnk/Utilities/
usr/X11R6/bin            := $(DESTDIR)/usr/X11R6/bin/
lib                      := $(DESTDIR)/lib/
usr/lib                  := $(DESTDIR)/usr/lib/
usr/include              := $(DESTDIR)/usr/include/

#SKIP     := $(bin)kill $(man1)kill.1

BINFILES := $(usr/bin)uptime $(usr/bin)tload $(usr/bin)free $(usr/bin)w \
            $(usr/bin)top $(usr/bin)vmstat $(usr/bin)watch $(usr/bin)skill \
            $(usr/bin)snice $(bin)kill $(sbin)sysctl $(usr/bin)pmap \
            $(usr/proc/bin)pgrep $(usr/proc/bin)pkill

MANFILES := $(man1)uptime.1 $(man1)tload.1 $(man1)free.1 $(man1)w.1 \
            $(man1)top.1 $(man1)watch.1 $(man1)skill.1 $(man1)kill.1 \
            $(man1)snice.1 $(man1)pgrep.1 $(man1)pkill.1 $(man1)pmap.1 \
            $(man5)sysctl.conf.5 $(man8)vmstat.8 $(man8)sysctl.8

TARFILES := AUTHORS BUGS NEWS README TODO COPYING COPYING.LIB \
            Makefile procps.lsm procps.spec v t README.top \
            minimal.c $(notdir $(MANFILES)) \
            uptime.c tload.c free.c w.c top.c vmstat.c watch.c skill.c \
            sysctl.c pgrep.c top.h pmap.c

CURSES := -I/usr/include/ncurses -lncurses

LDFLAGS := -Wl,-warn-common

CFLAGS := -D_GNU_SOURCE -O2 -g3 -fno-common -ffast-math -I proc \
  -W -Wall -Wshadow -Wcast-align -Wredundant-decls \
  -Wbad-function-cast -Wcast-qual -Wwrite-strings -Waggregate-return \
#  -Wpadded -Wunreachable-code -Wdisabled-optimization \
  -Wstrict-prototypes -Wmissing-prototypes # -Wconversion

############ misc.

# free.c pmap.c sysctl.c uptime.c vmstat.c watch.c pgrep.c skill.c tload.c top.c w.c
# utmp.c oldtop.c tmp-junk.c minimal.c

.SUFFIXES:
.SUFFIXES: .a .o .c .s .h

.PHONY: all clean do_all install tar  # ps

ALL := $(notdir $(BINFILES))

CLEAN := $(notdir $(BINFILES))

DIRS :=

INSTALL := $(BINFILES) $(MANFILES)

# want this rule first, use := on ALL, and ALL not filled in yet
all: do_all

-include */module.mk

do_all:    $(ALL)

junk := DEADJOE *~ *.o core gmon.out

# Remove $(junk) from all $(DIRS)
CLEAN += $(junk) $(foreach dir,$(DIRS),$(addprefix $(dir), $(junk)))

##########
# not maintained because it isn't really needed:
#
#SRC :=
#OBJ := $(patsubst %.c,%.o, $(filter %.c,$(SRC)))
#
#ifneq ($(MAKECMDGOALS),clean)
#-include $(OBJ:.o=.d)
#endif
#
#%.d: %.c
#	depend.sh $(CFLAGS) $< > $@
############

# don't want to type "make procps-$(TARVERSION).tar.gz"
tar: $(TARFILES)
	mkdir procps-$(TARVERSION)
	(tar cf - $(TARFILES)) | (cd procps-$(TARVERSION) && tar xf -)
	tar cf procps-$(TARVERSION).tar procps-$(TARVERSION)
	gzip -9 procps-$(TARVERSION).tar

clean:
	rm -f $(CLEAN)

###### install

$(BINFILES) : all
	$(install) --mode a=rx --strip $(notdir $@) $@

$(MANFILES) : all
	$(install) --mode a=r $(notdir $@) $@

install: $(filter-out $(SKIP),$(INSTALL))
	cd $(usr/bin) && ($(ln_f) skill snice; $(ln_f) pgrep pkill)

############ prog.c --> prog.o

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $^

w.o:    w.c
	$(CC) $(CFLAGS) $(W_SHOWFROM) -c $<

############ prog.o --> prog

pmap w uptime tload free sysctl vmstat utmp pgrep skill: % : %.o $(LIBPROC)
	$(CC) $(LDFLAGS) -o $@ $^

top:   % : %.o $(LIBPROC)
	$(CC) $(LDFLAGS) -o $@ $^ $(CURSES)

watch: % : %.o
	$(CC) $(LDFLAGS) -o $@ $^ $(CURSES)

############ progX --> progY

snice kill: skill
	ln -f skill $@

pkill: pgrep
	ln -f pgrep pkill
