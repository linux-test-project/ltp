# Copyright (c) 2019 - Linaro Limited. All rights reserved.
# SPDX-License-Identifier: GPL-2.0-or-later

top_srcdir		?= ../../../..

LTPLIBS = vdso

include $(top_srcdir)/include/mk/testcases.mk

LDLIBS+=-lrt
clock_gettime04: LTPLDLIBS = -lltpvdso

include $(top_srcdir)/include/mk/generic_leaf_target.mk
