# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2009, Cisco Systems Inc.

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

CPPFLAGS		+= -D_GNU_SOURCE
LDLIBS			+= $(AIO_LIBS)

include $(top_srcdir)/include/mk/generic_leaf_target.mk
