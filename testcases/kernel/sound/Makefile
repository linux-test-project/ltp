# SPDX-License-Identifier: GPL-2.0-or-later

top_srcdir		:= ../../..

include $(top_srcdir)/include/mk/testcases.mk

CPPFLAGS		+= -D_GNU_SOURCE

snd_timer01: CFLAGS+=-pthread
snd_timer01: LDLIBS+=-lrt
snd_seq01:	CFLAGS += -pthread
snd_seq01:	LDLIBS += -lrt

include $(top_srcdir)/include/mk/generic_leaf_target.mk
