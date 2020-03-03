# SPDX-License-Identifier: GPL-2.0-or-later
#  Copyright (c) SWSoft, 2007

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk
inotify09: CFLAGS+=-pthread
inotify09: LDLIBS+=-lrt
include $(top_srcdir)/include/mk/generic_leaf_target.mk
