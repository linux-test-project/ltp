# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2009, Cisco Systems Inc.
# Copyright (C) 2010, Linux Test Project.
# Ngie Cooper, July 2009

top_srcdir		?= ../../..

include $(top_srcdir)/include/mk/env_pre.mk

ifneq ($(WITH_EXPECT),yes)
FILTER_OUT_SUBDIRS	:= ftp
endif

INSTALL_DEPS		:= ../datafiles/bin.sm

../datafiles/bin.sm:
	$(MAKE) -C .. -f "$(abs_srcdir)/../Makefile" generate

include $(top_srcdir)/include/mk/generic_trunk_target.mk
