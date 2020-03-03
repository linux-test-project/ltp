# SPDX-License-Identifier: GPL-2.0-or-later
#    kernel/syscalls/ipc testcase suite common definitions Makefile.
#
#    Copyright (C) 2016, Cyril Hrubis
#

LDLIBS			+= -lnewipc
LIBDIR			:= ../libnewipc

LIBIPC			:= $(LIBDIR)/libnewipc.a

$(LIBDIR):
	mkdir -p "$@"

$(LIBIPC): $(LIBDIR)
	$(MAKE) -C $^ -f "$(abs_srcdir)/$^/Makefile" all

MAKE_DEPS		:= $(LIBIPC)

CPPFLAGS		+= -I$(abs_srcdir)/$(LIBDIR)

LDFLAGS			+= -L$(abs_builddir)/$(LIBDIR)
