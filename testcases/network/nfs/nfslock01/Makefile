# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2009, Cisco Systems Inc.
# Ngie Cooper, July 2009

top_srcdir			?= ../../../..

include $(top_srcdir)/include/mk/env_pre.mk

FILTER_OUT_MAKE_TARGETS		:= nfs_flock_func

INSTALL_TARGETS			:= *.sh

include $(top_srcdir)/include/mk/generic_leaf_target.mk

$(MAKE_TARGETS): %: nfs_flock_func.o
