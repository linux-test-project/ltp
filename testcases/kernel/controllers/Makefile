# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2009, Cisco Systems Inc.
# Ngie Cooper, July 2009

top_srcdir		?= ../../..

include $(top_srcdir)/include/mk/env_pre.mk

INSTALL_TARGETS		:= *.sh
LIBDIR			:= libcontrollers
LIB			:= $(LIBDIR)/libcontrollers.a
FILTER_OUT_DIRS		:= $(LIBDIR)

ifneq ($(HAVE_FTS_H),1)
	FILTER_OUT_DIRS += cpuset
endif

$(LIBDIR):
	mkdir -p "$@"

$(LIB): $(LIBDIR)
	$(MAKE) -C $^ -f "$(abs_srcdir)/$^/Makefile" all

MAKE_DEPS		:= $(LIB)

trunk-clean:: | lib-clean

lib-clean:: $(LIBDIR)
	$(MAKE) -C $^ -f "$(abs_srcdir)/$^/Makefile" clean

include $(top_srcdir)/include/mk/generic_trunk_target.mk
