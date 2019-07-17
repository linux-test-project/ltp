# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2001
# Copyright (c) 2019 Petr Vorel <petr.vorel@gmail.com>

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

CPPFLAGS += -DTEST_SETHOSTNAME

sethostname01: $(abs_srcdir)/../setdomainname/setdomainname01.c
	$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) $(OUTPUT_OPTION)

sethostname02: $(abs_srcdir)/../setdomainname/setdomainname02.c
	$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) $(OUTPUT_OPTION)

sethostname03: $(abs_srcdir)/../setdomainname/setdomainname03.c
	$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) $(OUTPUT_OPTION)

MAKE_TARGETS := sethostname01 sethostname02 sethostname03

include $(top_srcdir)/include/mk/generic_leaf_target.mk
