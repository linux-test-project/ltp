# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) Linux Test Project, 2010-2017
# Copyright (C) 2009, Cisco Systems Inc.
# Ngie Cooper, July 2009

ifeq ($(strip $(MANPREFIX)),)
$(error $$(MANPREFIX) not defined)
endif

include $(top_srcdir)/include/mk/env_pre.mk

INSTALL_DIR	:= $(mandir)/man$(MANPREFIX)

INSTALL_MODE    := 00644

INSTALL_TARGETS	?= *.$(MANPREFIX)

MAKE_TARGETS	:=

include $(top_srcdir)/include/mk/generic_leaf_target.mk
