# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2001

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

CPPFLAGS		+= -I$(abs_srcdir)/../utils

MAKE_TARGETS		:= $(patsubst $(abs_srcdir)/%.c,%,$(wildcard $(abs_srcdir)/*[0-9].c))

$(MAKE_TARGETS): %: move_pages_support.o

LDLIBS			+= -lpthread -lrt

include $(top_srcdir)/testcases/kernel/include/lib.mk
include $(top_srcdir)/include/mk/generic_leaf_target.mk
