#
#    testcases Makefile.
#
#    Copyright (C) 2009, Cisco Systems Inc.
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along
#    with this program; if not, write to the Free Software Foundation, Inc.,
#    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# Ngie Cooper, July 2009
#

top_srcdir		?= ..

include $(top_srcdir)/include/mk/env_pre.mk

# XXX (garrcoop):
# 1. kdump shouldn't be compiled by default, because it's runtime based and
#    WILL crash the build host (the tests need to be fixed to just build, not
#    run).
FILTER_OUT_DIRS		:= kdump

ifneq ($(WITH_OPEN_POSIX_TESTSUITE),yes)
FILTER_OUT_DIRS		+= open_posix_testsuite
endif

ifneq ($(WITH_REALTIME_TESTSUITE),yes)
FILTER_OUT_DIRS		+= realtime
endif

include $(top_srcdir)/include/mk/generic_trunk_target.mk
