#
#    kernel test suite Makefile.
#
#    Copyright (C) 2009, Cisco Systems Inc.
#    Copyright (C) 2010, Linux Test Project.
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

top_srcdir		?= ../..

include $(top_srcdir)/include/mk/env_pre.mk

# NOTE (garrcoop): mce-test isn't integrated into the build.

# Build syscalls in all scenarios.
SUBDIRS			:= syscalls
ifneq ($(UCLINUX),1)
# KEEP THIS LIST ALPHABETIZED PLEASE!
SUBDIRS			+= connectors \
			   containers \
			   controllers \
			   fs \
			   hotplug \
			   io \
			   ipc \
			   mem \
			   numa \
			   performance_counters \
			   pty \
			   sched \
			   security \
			   timers \
			   tracing \

ifeq ($(WITH_POWER_MANAGEMENT_TESTSUITE),yes)
SUBDIRS			+= power_management
endif

endif

MAKE_DEPS		:= include/linux_syscall_numbers.h

include:
	mkdir -p "$@"

linux-syscall-numbers-clean:: include
	$(MAKE) -C $^ -f "$(abs_srcdir)/$^/Makefile" clean

include/linux_syscall_numbers.h: include
	$(MAKE) -C $^ -f "$(abs_srcdir)/$^/Makefile" all

include $(top_srcdir)/include/mk/generic_trunk_target.mk
