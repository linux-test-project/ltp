# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2009, Cisco Systems Inc.
# Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
# Ngie Cooper, July 2009

top_srcdir		?= ../..

include $(top_srcdir)/include/mk/env_pre.mk

DIR			:= datafiles

CLEAN_TARGETS		+= $(DIR)

INSTALL_DIR		:= testcases/bin

INSTALL_TARGETS		:= $(addprefix $(DIR)/ascii.,sm med lg jmb)

RM			+= -r

ifeq ($(ANDROID),1)
FILTER_OUT_DIRS		+= lib6 rpc sockets
endif

ifeq ($(HAVE_RPC),)
FILTER_OUT_DIRS		+= rpc
endif

$(INSTALL_TARGETS): | generate

.PHONY: generate

include $(top_srcdir)/include/mk/generic_trunk_target.mk

$(DESTDIR)/$(INSTALL_DIR)/$(DIR) $(DIR): %:
	mkdir -p "$@"

generate: $(DIR)
	cd "$(DIR)" && \
	abs_top_srcdir="$(abs_top_srcdir)" "$(abs_srcdir)/generate.sh"
