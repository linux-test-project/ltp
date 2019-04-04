# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
# Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
#

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

include $(abs_srcdir)/../utils/newer_64.mk

%_64: CPPFLAGS += -D_FILE_OFFSET_BITS=64

preadv203: CFLAGS += -pthread
preadv203_64: CFLAGS += -pthread
preadv203_64: LDFLAGS += -pthread

include $(top_srcdir)/include/mk/generic_leaf_target.mk
