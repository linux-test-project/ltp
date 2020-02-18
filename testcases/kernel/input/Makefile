# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) SUSE 2015

top_srcdir			?= ../../..

include $(top_srcdir)/include/mk/testcases.mk

FILTER_OUT_MAKE_TARGETS		:= input_helper

include $(top_srcdir)/include/mk/generic_leaf_target.mk

$(MAKE_TARGETS): %: input_helper.o
