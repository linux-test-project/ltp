# SPDX-License-Identifier: GPL-2.0-or-later
top_srcdir		?= ../..

include $(top_srcdir)/include/mk/env_pre.mk

CFLAGS			+= -W -Wall
LDLIBS			+= -lltp

test08 test09 test15 tst_fuzzy_sync01 tst_fuzzy_sync02 tst_fuzzy_sync03: CFLAGS += -pthread
tst_expiration_timer tst_fuzzy_sync01 tst_fuzzy_sync02 tst_fuzzy_sync03: LDLIBS += -lrt

ifeq ($(ANDROID),1)
FILTER_OUT_MAKE_TARGETS	+= test08
endif


include $(top_srcdir)/include/mk/generic_leaf_target.mk
