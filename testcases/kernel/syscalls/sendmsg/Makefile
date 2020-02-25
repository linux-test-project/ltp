# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2001

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

CPPFLAGS		+= -I$(abs_srcdir)/../utils

sendmsg03:  CFLAGS += -pthread
sendmsg03:  LDLIBS += -pthread -lrt

include $(top_srcdir)/include/mk/generic_leaf_target.mk
