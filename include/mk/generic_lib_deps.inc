# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) Linux Test Project, 2026

ifeq ($(MAKELEVEL),0)

ifneq ($(filter $(abs_top_builddir)/testcases%,$(CURDIR)),)

MAKE_DEPS += $(abs_top_builddir)/lib/libltp.a
MAKE_DEPS += $(abs_top_builddir)/libs-all

.PHONY: $(abs_top_builddir)/lib/libltp.a
$(abs_top_builddir)/lib/libltp.a:
	mkdir -m 00755 -p "$(@D)"
	$(MAKE) -C "$(@D)" -f "$(abs_top_srcdir)/lib/Makefile" all

.PHONY: $(abs_top_builddir)/libs-all
$(abs_top_builddir)/libs-all: $(LTPLIBS_FILES) | $(abs_top_builddir)/lib/libltp.a
	mkdir -m 00755 -p "$(abs_top_builddir)/libs"
	$(MAKE) -C "$(abs_top_builddir)/libs" -f "$(abs_top_srcdir)/libs/Makefile" all

endif

ifneq ($(filter $(abs_top_builddir)/testcases $(abs_top_builddir)/testcases/kernel%,$(filter-out $(abs_top_builddir)/testcases/kernel/lib,$(CURDIR))),)

MAKE_DEPS += $(abs_top_builddir)/testcases/kernel/lib/libkerntest.a

.PHONY: $(abs_top_builddir)/testcases/kernel/lib/libkerntest.a
$(abs_top_builddir)/testcases/kernel/lib/libkerntest.a: | $(abs_top_builddir)/lib/libltp.a
	mkdir -m 00755 -p "$(@D)"
	$(MAKE) -C "$(@D)" -f "$(abs_top_srcdir)/testcases/kernel/lib/Makefile" all

endif

endif
