# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2022 Linux Test Project

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk
dirtyc0w_shmem_child: CFLAGS+=-pthread
include $(top_srcdir)/include/mk/generic_leaf_target.mk
