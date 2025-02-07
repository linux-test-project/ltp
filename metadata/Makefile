# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2020 Cyril Hrubis <chrubis@suse.cz>

top_srcdir		?= ..

include $(top_srcdir)/include/mk/env_pre.mk
include $(top_srcdir)/include/mk/functions.mk

MAKE_TARGETS		:= ltp.json
HOST_MAKE_TARGETS	:= metaparse
INSTALL_DIR		= metadata

.PHONY: ltp.json

ltp.json: metaparse
	$(abs_srcdir)/parse.sh > ltp.json

test:
	$(MAKE) -C $(abs_srcdir)/tests/ test

include $(top_srcdir)/include/mk/generic_leaf_target.mk
