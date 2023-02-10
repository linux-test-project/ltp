# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk
include $(abs_srcdir)/../Makefile.inc

LDLIBS			:= -lclone $(LDLIBS)

include $(top_srcdir)/include/mk/generic_leaf_target.mk
