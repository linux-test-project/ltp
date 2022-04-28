# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2001

top_srcdir              ?= ../../../../..

LTPLIBS = ltpipc ltpnewipc

include $(top_srcdir)/include/mk/testcases.mk

shmdt01: LTPLDLIBS = -lltpnewipc
shmdt02: LTPLDLIBS = -lltpipc

include $(top_srcdir)/include/mk/generic_leaf_target.mk
