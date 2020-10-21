# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2001

top_srcdir              ?= ../../../../..

LTPLIBS = ltpipc

include $(top_srcdir)/include/mk/testcases.mk
FILTER_OUT_MAKE_TARGETS         := msgstress_common
LTPLDLIBS  += -lltpipc

include $(top_srcdir)/include/mk/generic_leaf_target.mk
