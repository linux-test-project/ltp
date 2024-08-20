# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

landlock07: LDLIBS += $(KEYUTILS_LIBS)

include $(top_srcdir)/include/mk/generic_leaf_target.mk
