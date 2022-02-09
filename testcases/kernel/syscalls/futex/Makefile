# SPDX-License-Identifier: GPL-2.0-or-later
#  Copyright (c) 2015 Cyril Hrubis <chrubis@suse.cz>

top_srcdir		?= ../../../..

futex_cmp_requeue01: LDLIBS+=-lrt
futex_cmp_requeue02: LDLIBS+=-lrt
futex_wait02: LDLIBS+=-lrt
futex_wake03: LDLIBS+=-lrt
futex_wait05: LDLIBS+=-lrt
futex_wait_bitset01: LDLIBS+=-lrt

futex_wait03: CFLAGS+=-pthread
futex_wake02: CFLAGS+=-pthread
futex_wake04: CFLAGS+=-pthread
futex_waitv02: CFLAGS+=-pthread
futex_waitv03: CFLAGS+=-pthread

include $(top_srcdir)/include/mk/testcases.mk
include $(top_srcdir)/include/mk/generic_leaf_target.mk
