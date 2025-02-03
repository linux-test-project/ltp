# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) Linux Test Project, 2009-2024
# Ngie Cooper, July 2009

include $(top_srcdir)/include/mk/env_pre.mk
include $(top_srcdir)/include/mk/functions.mk
include $(top_srcdir)/include/mk/sparse.mk

APICMDS_DIR	:= $(abs_top_builddir)/tools/apicmds

LIBLTP_DIR	:= $(abs_top_builddir)/lib

LIBLTP		:= $(LIBLTP_DIR)/libltp.a

$(APICMDS_DIR)/tst_kvercmp: $(APICMDS_DIR)
	$(MAKE) -C "$^" -f "$(abs_top_srcdir)/tools/apicmds/Makefile" all

$(LIBLTP): $(LIBLTP_DIR)
	$(MAKE) -C "$^" -f "$(abs_top_srcdir)/lib/Makefile" all

MAKE_DEPS	:= $(LIBLTP)

INSTALL_DIR	:= testcases/bin

LDLIBS		+= -lltp

ifdef LTPLIBS

LTPLIBS_DIRS = $(addprefix $(abs_top_builddir)/libs/, $(LTPLIBS))
LTPLIBS_FILES = $(addsuffix .a, $(addprefix $(abs_top_builddir)/libs/, $(foreach LIB,$(LTPLIBS),$(LIB)/lib$(LIB))))

MAKE_DEPS += $(LTPLIBS_FILES)

.PHONY: $(LTPLIBS_FILES)

$(LTPLIBS_FILES): $(LTPLIBS_DIRS)

$(LTPLIBS_FILES): %:
ifdef VERBOSE
	$(MAKE) -C "$(dir $@)" -f "$(subst $(abs_top_builddir),$(abs_top_srcdir),$(dir $@))/Makefile" all
else
	@echo "BUILD $(notdir $@)"
	@$(MAKE) --no-print-directory -C "$(dir $@)" -f "$(subst $(abs_top_builddir),$(abs_top_srcdir),$(dir $@))/Makefile" all
endif

LDFLAGS += $(addprefix -L$(top_builddir)/libs/, $(LTPLIBS))

endif

$(LTPLIBS_DIRS) $(APICMDS_DIR) $(LIBLTP_DIR): %:
	mkdir -p "$@"
