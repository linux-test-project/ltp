# SPDX-License-Identifier: GPL-2.0-or-later

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

can_bcm01: CFLAGS += -pthread
can_bcm01: LDLIBS += -lrt

include $(top_srcdir)/include/mk/generic_leaf_target.mk
