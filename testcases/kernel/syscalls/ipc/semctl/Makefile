# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2001

top_srcdir              ?= ../../../../..

LTPLIBS = ltpipc ltpnewipc

include $(top_srcdir)/include/mk/testcases.mk

semctl01 semctl02 semctl03 semctl04 semctl05 semctl06 semctl07: LTPLDLIBS = -lltpipc
semctl08: LTPLDLIBS = -lltpnewipc

include $(top_srcdir)/include/mk/generic_leaf_target.mk
