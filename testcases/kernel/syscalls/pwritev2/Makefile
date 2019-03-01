# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
# Author: Jinhui Huang <huangjh.jy@cn.fujitsu.com>

top_srcdir              ?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

include $(abs_srcdir)/../utils/newer_64.mk

%_64: CPPFLAGS += -D_FILE_OFFSET_BITS=64

include $(top_srcdir)/include/mk/generic_leaf_target.mk
