#
#    kernel/security/selinux-testsuite/tests Makefile
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
# Garrett Cooper, August 2009
#

top_srcdir		?= ../../../../..

include	$(top_srcdir)/include/mk/env_pre.mk

ARGS			= -m
# Don't want to pass the -d option unless DESTDIR is a non-zero length string.
ifneq ($(strip $(DESTDIR)),)
ARGS			+= -d $(DESTDIR)
endif

DISTRO_VER		:= $(shell $(top_srcdir)/scripts/detect_distro.sh $(ARGS))

#
# Certain tests should be excluded on RHEL [45].x as their kernels don't
# support the tests.
#
# XXX (garrcoop): actually complete the work to add proper checks to the tests
# to report TCONF on configuration failure.

# RHEL 4.x and 5.x don't support the bounds test.
ifneq ($(findstring $(DISTRO_VER),redhat-4 redhat-5),)
FILTER_OUT_DIRS		+= bounds
endif
# RHEL 4.x doesn't support the dyntrace and dyntrans tests.
ifeq (redhat-5,$(DISTRO_VER))
FILTER_OUT_DIRS		+= dyntrace dyntrans
endif

include $(top_srcdir)/include/mk/generic_trunk_target.mk
