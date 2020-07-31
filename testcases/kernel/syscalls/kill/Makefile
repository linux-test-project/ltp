# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2001

top_srcdir		?= ../../../..

LTPLIBS = ltpipc

include $(top_srcdir)/include/mk/testcases.mk

kill07: LTPLDLIBS  = -lltpipc
kill05: LTPLDLIBS  = -lltpipc

include $(top_srcdir)/include/mk/generic_leaf_target.mk
