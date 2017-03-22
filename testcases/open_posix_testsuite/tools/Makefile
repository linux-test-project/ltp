#
# Read COPYING for licensing details.
#
# Ngie Cooper, June 2010
#

-include ../../../include/mk/config-openposix.mk

top_srcdir?=	..

srcdir=		$(top_srcdir)/tools

all: ../bin/t0

clean:
	@rm -f ../bin/t0

../bin:
	mkdir $@

../bin/t0: ../bin $(srcdir)/t0.c
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(srcdir)/t0.c $(LDLIBS)
