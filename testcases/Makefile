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
# Garrett Cooper, July 2009
#

top_srcdir		?= ..

include $(top_srcdir)/include/mk/env_pre.mk

# XXX (garrcoop):
# 1. ballista is broken (working on it).
# 2. kdump shouldn't be compiled by default, because it's runtime based and
#    WILL crash the build host (the tests need to be fixed to just build, not
#    run).
# 3. mce-test doesn't fit cleanly in the build, yet.
FILTER_OUT_DIRS		:= ballista kdump mce-test open%

ifneq ($(WITH_REALTIME_TESTSUITE),yes)
FILTER_OUT_DIRS		+= realtime
endif

#
# XXX (garrcoop): disabling the open_posix_testsuite by request from Subrata
# due to build times (open_posix_testsuite runs serially by design to
# deterministically catch build errors -- see
# testcases/open_posix_testsuite/Makefile for more details).
#
#trunk-all: open_posix_testsuite-all
#
#trunk-clean:: open_posix_testsuite-clean
#
#trunk-install: open_posix_testsuite-install

open_posix_testsuite:
	mkdir -p $@

open_posix_testsuite-%: open_posix_testsuite
	$(MAKE) -C $^ top_srcdir=$(abs_top_srcdir)/testcases/$^ $*

include $(top_srcdir)/include/mk/generic_trunk_target.mk
