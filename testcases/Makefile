# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2009, Cisco Systems Inc.
# Ngie Cooper, July 2009

top_srcdir		?= ..

include $(top_srcdir)/include/mk/env_pre.mk

# XXX (garrcoop):
# kdump shouldn't be compiled by default, because it's runtime based and will
# crash the build host (the tests need to be fixed to just build, not run).
FILTER_OUT_DIRS		:= kdump

ifneq ($(WITH_OPEN_POSIX_TESTSUITE),yes)
FILTER_OUT_DIRS		+= open_posix_testsuite
endif

include $(top_srcdir)/include/mk/generic_trunk_target.mk
