# This file gets included into the main Makefile, in the top directory.

INSTALL += $(bin)ps $(man1)ps.1

# files to remove
CLEAN += ps/ps ps/debug

# a directory for cleaning
DIRS += ps/

# a file to create
ALL += ps/ps

PS_C    := display global help output parser select sortformat
PSNAMES := $(addprefix ps/,$(PS_C))
PSOBJ   := $(addsuffix .o,$(PSNAMES))
PSSRC   := $(addsuffix .c,$(PSNAMES))

PS_X := COPYING HACKING TRANSLATION common.h module.mk it p ps.1 regression
TARFILES += $(PSSRC) $(addprefix ps/,$(PS_X))

ps/ps: $(PSOBJ) $(LIBPROC)
	$(CC) $(LDFLAGS) -o $@ $^

# This just adds the stacktrace code
ps/debug: $(PSOBJ) stacktrace.o $(LIBPROC)
	$(CC) -o $@ $^ -lefence

$(PSOBJ): %.o: %.c ps/common.h proc/$(SONAME)
	$(CC) -c $(CFLAGS) $< -o $@

ps/stacktrace.o: ps/stacktrace.c


$(bin)ps: ps/ps
	install --mode a=rx --strip $< $@

$(man1)ps.1 : ps/ps.1
	install --mode a=r $< $@
	-rm -f $(DESTDIR)/var/catman/cat1/ps.1.gz $(DESTDIR)/var/man/cat1/ps.1.gz
