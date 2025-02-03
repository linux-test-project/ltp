# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) Linux Test Project, 2021
# Rules to make sparse tool(s) for inclusion in lib and testcases Makefiles

SPARSE_DIR:= $(abs_top_builddir)/tools/sparse

$(SPARSE_DIR)/sparse-ltp: $(SPARSE_DIR)
	$(MAKE) -C "$^" all

$(SPARSE_DIR): %:
	mkdir -p "$@"
