# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019, Linux Test Project

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

CPPFLAGS += -DTEST_RT_SIGPENDING

rt_sigpending02: $(abs_srcdir)/../sigpending/sigpending02.c
	$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) $(OUTPUT_OPTION)

MAKE_TARGETS := rt_sigpending02

include $(top_srcdir)/include/mk/generic_leaf_target.mk
