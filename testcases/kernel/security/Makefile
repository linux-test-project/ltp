#
#    kernel/security test suite Makefile
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

top_srcdir		?= ../../..

include	$(top_srcdir)/include/mk/env_pre.mk

# For broken compilers and toolchains, like Montavista, that improperly detect
# system headers when running autoconf -_-... bleh.
ifeq ($(strip $(CAP_LIBS)),)
FILTER_OUT_DIRS		:= cap_bound filecaps
endif
ifeq ($(HAVE_SETCAP),false)
FILTER_OUT_DIRS		+= filecaps
endif

# XXX (garrcoop): avoid compilation failures on RHEL 5.4, as reported by
# Mitani-san, because of policy versioning issues...
ifeq (1,1)
FILTER_OUT_DIRS		+= selinux-testsuite
else
# Omit the selinux-testsuite if -lselinux is missing or the policy-devel
# package isn't present.
ifneq ($(strip $(SELINUX_LIBS)),)
FILTER_OUT_DIRS		+= selinux-testsuite
else
# NOTE (garrcoop): Keep this in sync with
# testcases/kernel/security/selinux-testsuite/refpolicy/Makefile
ifeq ($(wildcard $(DESTDIR)/usr/share/selinux/devel),)
FILTER_OUT_DIRS		+= selinux-testsuite
endif
endif
endif

OPT_CFLAGS		+= -O
DEBUG_LDFLAGS		+= -s

include $(top_srcdir)/include/mk/generic_trunk_target.mk
