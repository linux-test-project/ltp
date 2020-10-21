# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2001

top_srcdir              ?= ../../../../..

LTPLIBS = ltpnewipc

include $(top_srcdir)/include/mk/testcases.mk

LTPLDLIBS  = -lltpnewipc
FILTER_OUT_MAKE_TARGETS         := msgstress_common
MAKE_TARGETS            := $(patsubst $(abs_srcdir)/%.c,%,$(wildcard $(abs_srcdir)/*[1-4].c))
$(MAKE_TARGETS): %: msgstress_common.o

include $(top_srcdir)/include/mk/generic_leaf_target.mk
