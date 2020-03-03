# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2001

top_srcdir              ?= ../../../../..

shmctl05: CFLAGS += -pthread
shmctl05: LDLIBS += -lrt

include $(top_srcdir)/include/mk/testcases.mk
include $(abs_srcdir)/../Makefile.inc
include $(top_srcdir)/include/mk/generic_leaf_target.mk
