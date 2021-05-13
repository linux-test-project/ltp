# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2009, Cisco Systems Inc.
# Ngie Cooper, July 2009

top_srcdir		?= ../..

include $(top_srcdir)/include/mk/env_pre.mk

LIBSCTPDIR		:= lib
LIBSCTPUTILDIR		:= testlib
LIBSCTP			:= $(LIBSCTPDIR)/libsctp.a
LIBSCTPUTIL		:= $(LIBSCTPUTILDIR)/libsctputil.a

FILTER_OUT_DIRS		:= $(LIBSCTPDIR) $(LIBSCTPUTILDIR)

$(LIBSCTPDIR) $(LIBSCTPUTILDIR):
	mkdir -p "$@"

lib-clean:: $(LIBSCTPDIR) $(LIBSCTPUTILDIR)
	for i in $^; do \
	    $(MAKE) -C "$$i" -f "$(abs_srcdir)/$$i/Makefile" clean; \
	done


$(LIBSCTP): $(LIBSCTPDIR)
	$(MAKE) -C "$^" -f "$(abs_srcdir)/$^/Makefile" all

$(LIBSCTPUTIL): $(LIBSCTPUTILDIR)
	$(MAKE) -C "$^" -f "$(abs_srcdir)/$^/Makefile" all

trunk-all: $(LIBSCTP) $(LIBSCTPUTIL)

trunk-clean:: | lib-clean

include $(top_srcdir)/include/mk/generic_trunk_target.mk
