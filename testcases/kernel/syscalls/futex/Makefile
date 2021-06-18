# SPDX-License-Identifier: GPL-2.0-or-later
#  Copyright (c) 2015 Cyril Hrubis <chrubis@suse.cz>

top_srcdir		?= ../../../..

futex_cmp_requeue01 futex_cmp_requeue02 futex_wait02 futex_wake03 futex_wait05 futex_wait_bitset01: LDLIBS += -lrt
futex_wait03 futex_wake02 futex_wake04: CFLAGS += -pthread

include $(top_srcdir)/include/mk/testcases.mk
include $(top_srcdir)/include/mk/generic_leaf_target.mk
