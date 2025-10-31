# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2023 SUSE LLC <andrea.cervesato@suse.com>
#
# Install script for Linux Testing Project

top_srcdir		?= ../..

include $(top_srcdir)/include/mk/env_pre.mk

BASE_DIR		:= $(abspath $(DESTDIR)/$(prefix))

install:
	mkdir -p $(BASE_DIR)/libkirk
	mkdir -p $(BASE_DIR)/libkirk/channels

	install -m 00644 $(top_srcdir)/tools/kirk/kirk-src/libkirk/*.py $(BASE_DIR)/libkirk
	install -m 00644 $(top_srcdir)/tools/kirk/kirk-src/libkirk/channels/*.py $(BASE_DIR)/libkirk/channels
	install -m 00775 $(top_srcdir)/tools/kirk/kirk-src/kirk $(BASE_DIR)/kirk

	cd $(BASE_DIR) && ln -sf kirk runltp-ng

include $(top_srcdir)/include/mk/generic_leaf_target.mk
