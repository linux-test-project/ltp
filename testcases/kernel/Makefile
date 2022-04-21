# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2009, Cisco Systems Inc.
# Copyright (C) 2010, Linux Test Project.
# Ngie Cooper, July 2009

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
			   irq \
			   kvm \
			   logging \
			   mem \
			   numa \
			   pty \
			   sched \
			   security \
			   sound \
			   tracing \
			   uevents \
			   watchqueue \

ifeq ($(WITH_POWER_MANAGEMENT_TESTSUITE),yes)
SUBDIRS			+= power_management
endif

endif

ifeq ($(ANDROID),1)
FILTER_OUT_DIRS		+= containers controllers device-drivers fs io ipc mem \
				sched security
endif

include $(top_srcdir)/include/mk/generic_trunk_target.mk
