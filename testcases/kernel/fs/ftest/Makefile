# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) 2009, Cisco Systems Inc.
# Copyright (C) Ngie Cooper, July 2009

top_srcdir			?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

FILTER_OUT_MAKE_TARGETS		:= libftest

include $(top_srcdir)/include/mk/generic_leaf_target.mk

$(MAKE_TARGETS): %: libftest.o
