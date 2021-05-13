# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2009, Cisco Systems Inc.
# Ngie Cooper, July 2009

top_srcdir		?= ../../..

include $(top_srcdir)/include/mk/env_pre.mk

ifeq ($(ANDROID), 1)
FILTER_OUT_DIRS		+= ns-tools
endif

include $(top_srcdir)/include/mk/generic_trunk_target.mk
