# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2001

top_srcdir		?= ../../../..
setsockopt06 setsockopt07:	CFLAGS += -pthread
setsockopt06 setsockopt07:	LDLIBS += -lrt

include $(top_srcdir)/include/mk/testcases.mk

include $(top_srcdir)/include/mk/generic_leaf_target.mk
