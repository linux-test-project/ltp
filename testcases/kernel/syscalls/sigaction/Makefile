# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2001

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

CFLAGS			+= -DGLIBC_SIGACTION_BUG=1 -D_GNU_SOURCE

LDLIBS			+= -lpthread

include $(top_srcdir)/include/mk/generic_leaf_target.mk
