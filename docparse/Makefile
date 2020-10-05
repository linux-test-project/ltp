# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Cyril Hrubis <chrubis@suse.cz>

top_srcdir		?= ..

include $(top_srcdir)/include/mk/env_pre.mk
include $(top_srcdir)/include/mk/functions.mk

MAKE_TARGETS		:= metadata.json
HOST_MAKE_TARGETS	:= docparse

INSTALL_DIR = metadata

.PHONY: metadata.json

metadata.json: docparse
	$(abs_srcdir)/parse.sh > metadata.json

include $(top_srcdir)/include/mk/generic_leaf_target.mk
