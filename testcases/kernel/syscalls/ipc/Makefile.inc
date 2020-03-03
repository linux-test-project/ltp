# SPDX-License-Identifier: GPL-2.0-or-later
#    kernel/syscalls/ipc testcase suite common definitions Makefile.
#
#    Copyright (C) 2009, Cisco Systems Inc.
#
# Ngie Cooper, July 2009
#

# DO NOT USE THIS FILE FOR ipc / lib!!!

LDLIBS			+= -lipc
LIBDIR			:= ../lib

LIBIPC			:= $(LIBDIR)/libipc.a

$(LIBDIR):
	mkdir -p "$@"

$(LIBIPC): $(LIBDIR)
	$(MAKE) -C $^ -f "$(abs_srcdir)/$^/Makefile" all

MAKE_DEPS		:= $(LIBIPC)

CPPFLAGS		+= -I$(abs_srcdir)/$(LIBDIR)

LDFLAGS			+= -L$(abs_builddir)/$(LIBDIR)

# vim: syntax=make
