# SPDX-License-Identifier: GPL-2.0-or-later
#  Copyright (c) Zilogic Systems Pvt. Ltd., 2018
#  Email: code@zilogic.com

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

%_64: CPPFLAGS += -D_FILE_OFFSET_BITS=64

include $(top_srcdir)/include/mk/generic_leaf_target.mk

statx06: LDLIBS += -lrt
