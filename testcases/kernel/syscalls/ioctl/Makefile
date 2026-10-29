# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2001

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

INSTALL_TARGETS		+= test_ioctl

ioctl01: LDLIBS+=-lutil

ifeq ($(ANDROID),1)
FILTER_OUT_MAKE_TARGETS	+= ioctl02
endif

include $(top_srcdir)/include/mk/generic_leaf_target.mk
