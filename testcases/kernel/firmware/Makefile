# Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it would be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write the Free Software Foundation,
# Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

top_srcdir	?= ../../..

include $(top_srcdir)/include/mk/env_pre.mk

SUBDIRS			=
REQ_VERSION_MAJOR	= 3
REQ_VERSION_PATCH	= 7

ifeq ($(MAKECMDGOALS),clean)
SKIP = 0
endif

ifeq ($(WITH_MODULES),yes)
SKIP ?= $(shell test $(LINUX_VERSION_MAJOR) -gt $(REQ_VERSION_MAJOR); echo $$?)
ifeq ($(SKIP),1)
SKIP = $(shell test $(LINUX_VERSION_MAJOR) -eq $(REQ_VERSION_MAJOR); echo $$?)
ifeq ($(SKIP),0)
SKIP = $(shell test $(LINUX_VERSION_PATCH) -ge $(REQ_VERSION_PATCH); echo $$?)
endif
endif
endif

ifeq ($(SKIP),0)
SUBDIRS			+= fw_load_kernel
endif
SUBDIRS			+= fw_load_user

include $(top_srcdir)/include/mk/generic_trunk_target.mk
