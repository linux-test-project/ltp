# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2001

top_srcdir		?= ../../../..

include	$(top_srcdir)/include/mk/env_pre.mk

LIBDIR			:= lib
LIB			:= $(LIBDIR)/libhugetlb.a
FILTER_OUT_DIRS		:= $(LIBDIR)

$(LIBDIR):
	mkdir -p "$@"

$(LIB): $(LIBDIR)
	$(MAKE) -C $^ -f "$(abs_srcdir)/$^/Makefile" all

MAKE_DEPS		:= $(LIB)

trunk-clean:: | lib-clean

lib-clean:: $(LIBDIR)
	$(MAKE) -C $^ -f "$(abs_srcdir)/$^/Makefile" clean

include $(top_srcdir)/include/mk/generic_trunk_target.mk
