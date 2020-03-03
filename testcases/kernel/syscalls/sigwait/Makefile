# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2009

top_srcdir ?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

CPPFLAGS += -DTEST_SIGWAIT

sigwait01: $(abs_srcdir)/../sigwaitinfo/sigwaitinfo01.c
	$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) $(OUTPUT_OPTION)

MAKE_TARGETS := sigwait01

include $(top_srcdir)/include/mk/generic_leaf_target.mk
