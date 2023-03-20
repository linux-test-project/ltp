# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2001

top_srcdir              ?= ../../../../..

LTPLIBS = ltpipc ltpnewipc

include $(top_srcdir)/include/mk/testcases.mk

semget01 semget02: LTPLDLIBS = -lltpnewipc
semget05: LTPLDLIBS = -lltpipc

include $(top_srcdir)/include/mk/generic_leaf_target.mk
