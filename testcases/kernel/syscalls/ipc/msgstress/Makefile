# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2001

top_srcdir              ?= ../../../../..

include $(top_srcdir)/include/mk/testcases.mk

LIBMSGCTL               := $(LIBDIR)/libmsgctl.a
LDLIBS                  += -lmsgctl

MAKE_DEPS               := $(LIBMSGCTL)

include $(abs_srcdir)/../Makefile.inc
include $(top_srcdir)/include/mk/generic_leaf_target.mk
