# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2008
# Author: Veerendra <veeren@linux.vnet.ibm.com>
# Copyright (c) 2015 Red Hat, Inc.

top_srcdir              ?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk
include $(abs_srcdir)/../Makefile.inc

LDLIBS                  := -lclone -lltp

include $(top_srcdir)/include/mk/generic_leaf_target.mk
