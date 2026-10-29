# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2009

top_srcdir		?= ../../../..

LTPLIBS = numa

include $(top_srcdir)/include/mk/testcases.mk

CPPFLAGS		+= -I$(abs_srcdir)/../utils/

LDLIBS  += $(NUMA_LIBS)
LTPLDLIBS = -lltpnuma

include $(top_srcdir)/testcases/kernel/include/lib.mk
include $(top_srcdir)/include/mk/generic_leaf_target.mk
