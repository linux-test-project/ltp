# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2001

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk
include $(abs_srcdir)/../utils/newer_64.mk

LDLIBS			+= -lpthread -lrt

%_64: CPPFLAGS += -D_FILE_OFFSET_BITS=64

include $(top_srcdir)/include/mk/generic_leaf_target.mk

