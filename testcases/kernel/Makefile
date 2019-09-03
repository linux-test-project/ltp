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
# Ngie Cooper, July 2009
#

top_srcdir		?= ../..

include $(top_srcdir)/include/mk/env_pre.mk

# NOTE (garrcoop): mce-test isn't integrated into the build.

# Build syscalls in all scenarios.
SUBDIRS			:= syscalls

# Build lib
SUBDIRS			+= lib

ifneq ($(UCLINUX),1)
# KEEP THIS LIST ALPHABETIZED PLEASE!
SUBDIRS			+= connectors \
			   containers \
			   controllers \
			   crypto \
			   device-drivers \
			   firmware \
			   fs \
			   hotplug \
			   input \
			   io \
			   ipc \
			   logging \
			   mem \
			   numa \
			   pty \
			   sched \
			   security \
			   sound \
			   tracing \
			   uevents \

ifeq ($(WITH_POWER_MANAGEMENT_TESTSUITE),yes)
SUBDIRS			+= power_management
endif

endif

ifeq ($(ANDROID),1)
FILTER_OUT_DIRS		+= containers controllers device-drivers fs io ipc mem \
				sched security
endif

include $(top_srcdir)/include/mk/generic_trunk_target.mk
