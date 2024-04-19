# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) Linux Test Project, 2012-2020
# Copyright (c) International Business Machines  Corp., 2009

top_srcdir		?= ../../../..

LTPLIBS = numa

include $(top_srcdir)/include/mk/testcases.mk

LDLIBS  += $(NUMA_LIBS)
LTPLDLIBS = -lltpnuma

include $(top_srcdir)/include/mk/generic_leaf_target.mk
