# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2011  Red Hat, Inc.

top_srcdir              ?= ../../../..

vma05_vdso: CFLAGS+=-ggdb3

include $(top_srcdir)/include/mk/testcases.mk
include $(top_srcdir)/testcases/kernel/include/lib.mk

INSTALL_TARGETS		:= vma05.sh

include $(top_srcdir)/include/mk/generic_leaf_target.mk
