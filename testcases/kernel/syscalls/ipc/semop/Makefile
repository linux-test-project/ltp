# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2001

top_srcdir              ?= ../../../../..

LTPLIBS = newipc

include $(top_srcdir)/include/mk/testcases.mk

semop01: LTPLDLIBS  = -lltpnewipc
semop02: LTPLDLIBS  = -lltpnewipc
semop03: LTPLDLIBS  = -lltpnewipc
semop05: LDLIBS    += -lpthread

include $(top_srcdir)/include/mk/generic_leaf_target.mk
