#
# Read COPYING for licensing details.
#
# Ngie Cooper, June 2010
#

top_srcdir ?= ..
srcdir     =  $(top_srcdir)/tools

include ../include/mk/env.mk

.PHONY: all
all: ../bin/t0

.PHONY: clean
clean:
	@rm -f ../bin/t0

../bin:
	mkdir $@

../bin/t0: ../bin $(srcdir)/t0.c
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(srcdir)/t0.c $(LDLIBS)
