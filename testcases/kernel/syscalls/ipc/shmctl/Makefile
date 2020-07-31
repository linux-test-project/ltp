# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2001

top_srcdir              ?= ../../../../..

LTPLIBS = ltpipc ltpnewipc

shmctl05: CFLAGS += -pthread
shmctl05: LDLIBS += -lrt

include $(top_srcdir)/include/mk/testcases.mk

shmctl01 shmctl02 shmctl03 shmctl04 shmctl05: LTPLDLIBS = -lltpipc
shmctl06: LTPLDLIBS = -lltpnewipc

include $(top_srcdir)/include/mk/generic_leaf_target.mk
