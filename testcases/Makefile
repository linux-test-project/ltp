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
# 2. mce-test doesn't fit cleanly in the build, yet.
# 3. pounder21 is just broken.
FILTER_OUT_DIRS		:= ballista mce-test pounder21

# XXX (garrcoop): Not sure why realtime is still in this list; this was the
# original list of disabled items...
ifneq ($(UCLINUX),1)
FILTER_OUT_DIRS		+= audit-test DOTS kdump open% realtime
else
SUBDIRS			:= kernel
endif

include $(top_srcdir)/include/mk/generic_trunk_target.mk
