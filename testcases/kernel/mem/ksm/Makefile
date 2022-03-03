# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2010  Red Hat, Inc.

top_srcdir		?= ../../../..

LTPLIBS = ltpnuma
ksm06: LTPLDLIBS = -lltpnuma

include $(top_srcdir)/include/mk/testcases.mk
include $(top_srcdir)/testcases/kernel/mem/include/libmem.mk
include $(top_srcdir)/include/mk/generic_leaf_target.mk
