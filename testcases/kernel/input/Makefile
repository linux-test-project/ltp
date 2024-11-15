# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) SUSE 2015

top_srcdir			?= ../../..

LTPLIBS = uinput

include $(top_srcdir)/include/mk/testcases.mk

FILTER_OUT_MAKE_TARGETS		:= input_helper
input01 input02 input03 input04 input05: LDLIBS += -lltpuinput

input06: %: input_helper.o

include $(top_srcdir)/include/mk/generic_leaf_target.mk
