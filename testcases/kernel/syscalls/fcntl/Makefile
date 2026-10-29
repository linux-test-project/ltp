# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2001

top_srcdir		?= ../../../..

fcntl33: LDLIBS+=-lrt
fcntl33_64: LDLIBS+=-lrt

fcntl34: LDLIBS += -lpthread
fcntl34_64: LDLIBS += -lpthread

fcntl36: LDLIBS += -lpthread
fcntl36_64: LDLIBS += -lpthread

include $(top_srcdir)/include/mk/testcases.mk
include $(abs_srcdir)/../utils/newer_64.mk

%_64: CPPFLAGS += -D_FILE_OFFSET_BITS=64

CPPFLAGS		+= -D_GNU_SOURCE -D_LARGEFILE64_SOURCE

include $(top_srcdir)/include/mk/generic_leaf_target.mk
