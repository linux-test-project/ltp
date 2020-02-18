# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2017  Red Hat, Inc.

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

FILTER_OUT_MAKE_TARGETS         := memfd_create_common

include $(top_srcdir)/include/mk/generic_leaf_target.mk

$(MAKE_TARGETS): %: memfd_create_common.o
