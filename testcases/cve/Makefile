# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2017 Linux Test Project

top_srcdir		?= ../..

include $(top_srcdir)/include/mk/testcases.mk

CFLAGS			+= -D_GNU_SOURCE

stack_clash:	CFLAGS += -fno-optimize-sibling-calls -Wno-infinite-recursion

cve-2016-7042:	LDLIBS += $(KEYUTILS_LIBS)

cve-2014-0196 cve-2016-7117 cve-2017-2671 cve-2017-17052 cve-2017-17053:  CFLAGS += -pthread
cve-2014-0196 cve-2016-7117 cve-2017-2671:  LDLIBS += -lrt

ifneq ($(ANDROID),1)
cve-2014-0196:  LDLIBS += -lutil
endif

ifneq (,$(filter $(HOST_CPU),x86 x86_64))
meltdown: CFLAGS += -msse2
endif

cve-2015-3290:	CFLAGS += -pthread -fomit-frame-pointer

include $(top_srcdir)/include/mk/generic_leaf_target.mk
