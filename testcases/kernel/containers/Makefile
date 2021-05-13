# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2007

top_srcdir		?= ../../..

include $(top_srcdir)/include/mk/env_pre.mk

LIBDIR			:= libclone

FILTER_OUT_DIRS		:= $(LIBDIR)

LIB			:= $(LIBDIR)/libclone.a

LDLIBS			:= -ldl -lltp

INSTALL_TARGETS		:= *.sh

$(LIBDIR):
	mkdir -p "$@"

# Make the target the real lib so we don't have to deal with rebuilding this
# every time the dependency is evaluated, like with PHONY rules.
$(LIB): $(LIBDIR)
	$(MAKE) -C $^ -f "$(abs_srcdir)/$(LIBDIR)/Makefile" all

MAKE_DEPS		:= $(LIB)

trunk-clean:: | lib-clean

lib-clean:: $(LIBDIR)
	$(MAKE) -C $^ -f "$(abs_srcdir)/$(LIBDIR)/Makefile" clean

include $(top_srcdir)/include/mk/generic_trunk_target.mk
