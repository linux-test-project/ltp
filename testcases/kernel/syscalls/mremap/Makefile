# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2001

top_srcdir		?= ../../../..

LTPLIBS = ipc

include $(top_srcdir)/include/mk/testcases.mk

mremap04: LTPLDLIBS  = -lltpipc
mremap07: LDLIBS  += -lpthread

include $(top_srcdir)/include/mk/generic_leaf_target.mk
