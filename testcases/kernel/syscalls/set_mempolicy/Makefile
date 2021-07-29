# SPDX-License-Identifier: GPL-2.0-or-later
top_srcdir		?= ../../../..

LTPLIBS = ltpnuma

include $(top_srcdir)/include/mk/testcases.mk

NEEDS_LIBS = set_mempolicy01 set_mempolicy02 set_mempolicy03 set_mempolicy04

$(NEEDS_LIBS): LDLIBS  += $(NUMA_LIBS)
$(NEEDS_LIBS): LTPLDLIBS = -lltpnuma

include $(top_srcdir)/include/mk/generic_leaf_target.mk
