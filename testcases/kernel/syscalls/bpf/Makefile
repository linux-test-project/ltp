# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Linux Test Project

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

FILTER_OUT_MAKE_TARGETS	:= bpf_common
CFLAGS			+= -D_GNU_SOURCE

include $(top_srcdir)/include/mk/generic_leaf_target.mk

$(MAKE_TARGETS): %: bpf_common.o
