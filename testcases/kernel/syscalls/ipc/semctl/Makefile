# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2001

top_srcdir              ?= ../../../../..

LTPLIBS = ipc newipc

include $(top_srcdir)/include/mk/testcases.mk

semctl06: LTPLDLIBS = -lltpipc
semctl02 semctl03 semctl04 semctl05 semctl07 semctl08 semctl09: LTPLDLIBS = -lltpnewipc

include $(top_srcdir)/include/mk/generic_leaf_target.mk
