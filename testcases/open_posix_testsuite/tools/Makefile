#
# Read COPYING for licensing details.
#
# Garrett Cooper, June 2010
#

top_srcdir?=	$(abspath ../../..)

srcdir=		$(top_srcdir)/testcases/open_posix_testsuite/tools

all: ../bin/t0

clean:
	@rm -f ../bin/t0

../bin:
	mkdir $@

../bin/t0: ../bin $(srcdir)/t0.c
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(srcdir)/t0.c $(LDLIBS)
