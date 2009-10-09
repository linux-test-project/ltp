SHELL = /bin/sh

EXECS = scsimain

LARGE_FILE_FLAGS = -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64

CFLAGS = -g -O2 -Wall -D_REENTRANT $(LARGE_FILE_FLAGS)
# CFLAGS = -g -O2 -Wall -D_REENTRANT -DSG_KERNEL_INCLUDES $(LARGE_FILE_FLAGS)
# CFLAGS = -g -O2 -Wall -pedantic -D_REENTRANT $(LARGE_FILE_FLAGS)

LDFLAGS =

all: $(EXECS)

depend dep:
	@set -e; for i in *.c; do $(CC) $(INCLUDES) $(CFLAGS) -M $$i; \
	done > .depend

clean:
	/bin/rm -f *.o $(EXECS) core .depend

scsimain: scsimain.o sg_err.o llseek.o
	$(LD) -o $@ $(LDFLAGS) $^ -lpthread

install: $(EXECS)
	install -d $(INSTDIR)
	@set -e; for name in $^; \
	 do install -s -o root -g root -m 755 $$name $(INSTDIR); \
	done
	install -d $(MANDIR)/$(MAN_PREF)
	@set -e; for mp in $(MAN_PGS); \
	 do install -o root -g root -m 644 $$mp $(MANDIR)/$(MAN_PREF); \
	 gzip -9f $(MANDIR)/$(MAN_PREF)/$$mp; \
	done

uninstall:
	dists="$(EXECS)"; \
	@set -e; for name in $$dists; do \
	 rm -f $(INSTDIR)/$$name; \
	done
	@set -e; for mp in $(MAN_PGS); do \
	 rm -f $(MANDIR)/$(MAN_PREF)/$$mp.gz; \
	done

ifeq (.depend,$(wildcard .depend))
include .depend
endif
