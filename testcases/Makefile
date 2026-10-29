# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2009, Cisco Systems Inc.
# Ngie Cooper, July 2009

top_srcdir		?= ..

include $(top_srcdir)/include/mk/env_pre.mk

ifneq ($(WITH_OPEN_POSIX_TESTSUITE),yes)
FILTER_OUT_DIRS		+= open_posix_testsuite
endif

include $(top_srcdir)/include/mk/generic_trunk_target.mk
