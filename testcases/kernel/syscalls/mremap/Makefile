# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2001

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

LDLIBS			+= -lipc

LIBDIR			:= ../ipc/lib

LIB                     := $(LIBDIR)/libipc.a

$(LIBDIR):
	mkdir -p "$@"

$(LIB): $(LIBDIR)
	$(MAKE) -C $^ -f "$(abs_srcdir)/$^/Makefile" all

MAKE_DEPS		:= $(LIB)

CPPFLAGS                += -I$(abs_srcdir)/$(LIBDIR)

LDFLAGS                 += -L$(abs_builddir)/$(LIBDIR)

include $(top_srcdir)/include/mk/generic_leaf_target.mk
