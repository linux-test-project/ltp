# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2001

top_srcdir              ?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk
include $(top_srcdir)/include/mk/generic_leaf_target.mk

mmapstress01: CFLAGS += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
