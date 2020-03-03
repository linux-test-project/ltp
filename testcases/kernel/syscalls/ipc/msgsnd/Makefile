# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2001

top_srcdir              ?= ../../../../..

ifeq ($(UCLINUX),1)
FILTER_OUT_MAKE_TARGETS += msgsnd05 msgsnd06
endif

include $(top_srcdir)/include/mk/testcases.mk
include $(abs_srcdir)/../Makefile2.inc
include $(top_srcdir)/include/mk/generic_leaf_target.mk
