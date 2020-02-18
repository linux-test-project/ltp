# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2012 Linux Test Project, Inc.

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

MAKE_TARGETS		:= $(patsubst $(abs_srcdir)/%.c,%,$(wildcard $(abs_srcdir)/*[0-9].c))
$(MAKE_TARGETS): %: migrate_pages_common.o

CPPFLAGS		+= -I$(abs_srcdir)/../utils/

include $(top_srcdir)/testcases/kernel/include/lib.mk
include $(top_srcdir)/include/mk/generic_leaf_target.mk
