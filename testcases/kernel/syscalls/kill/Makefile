# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2001

top_srcdir		?= ../../../..

LTPLIBS = ipc newipc

include $(top_srcdir)/include/mk/testcases.mk

kill07: LTPLDLIBS  = -lltpipc
kill05: LTPLDLIBS  = -lltpnewipc

include $(top_srcdir)/include/mk/generic_leaf_target.mk
