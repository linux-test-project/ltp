# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Red Hat, Inc.

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

pkey01: CFLAGS += -falign-functions=64

include $(top_srcdir)/include/mk/generic_leaf_target.mk
