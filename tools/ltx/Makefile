# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2023 Cyril Hrubis <chrubis@suse.cz>
# Copyright (c) 2023 Andrea Cervesato <andrea.cervesato@suse.com>
#
# Install script for Linux Test eXecutor

top_srcdir		?= ../..

include $(top_srcdir)/include/mk/env_pre.mk

ifneq ($(wildcard $(abs_srcdir)/ltx-src/*),)

BINARY=ltx

MAKE_TARGETS := $(BINARY)

CFLAGS+=-I$(abs_srcdir)/ltx-src/ -I$(abs_srcdir)/ltx-src/msgpack/

$(BINARY): $(wildcard $(abs_srcdir)/ltx-src/*.c $(abs_srcdir)/ltx-src/msgpack/*.c)
ifdef VERBOSE
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@
else
	@echo CC $@
	@$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@
endif

INSTALL_DIR := $(prefix)

endif

include $(top_srcdir)/include/mk/generic_leaf_target.mk
