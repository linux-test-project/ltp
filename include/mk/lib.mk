# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) Linux Test Project, 2009-2019
# Copyright (C) Cyril Hrubis <chrubis@suse.cz> 2012
# Copyright (c) Cisco Systems Inc., 2009
# Ngie Cooper, July 2009

# Makefile to include for libraries.

include $(top_srcdir)/include/mk/env_pre.mk
include $(top_srcdir)/include/mk/sparse.mk

INSTALL_DIR	:= $(libdir)

# An extension of generic_leaf_target, strictly for libraries.
.PHONY: install_headers

ifndef LIB
ifndef INTERNAL_LIB
$(error You must define LIB or INTERNAL_LIB when including this Makefile)
endif
endif

install_headers: $(addprefix $(DESTDIR)/$(includedir)/,$(notdir $(HEADER_FILES)))

INSTALL_MODE	?= 00664

# Hide the LIB target for internal libs on install
ifneq ($(MAKECMDGOALS),install)
LIB ?= $(INTERNAL_LIB)
endif

MAKE_TARGETS	+= $(LIB)

LIBSRCS		?= $(wildcard $(abs_srcdir)/*.c)
LIBSRCS		:= $(sort $(LIBSRCS))
LIBSRCS		:= $(abspath $(LIBSRCS))
LIBSRCS		:= $(subst $(abs_srcdir)/,,$(wildcard $(LIBSRCS)))
LIBSRCS		:= $(filter-out $(FILTER_OUT_LIBSRCS),$(LIBSRCS))

LIBOBJS		:= $(LIBSRCS:.c=.o)

CHECK_TARGETS	:= $(addprefix check-,$(notdir $(LIBSRCS:.c=)))

$(LIB): $(notdir $(LIBOBJS))
	@if [ -z "$(strip $^)" ] ; then \
		echo "Cowardly refusing to create empty archive"; \
		exit 1; \
	fi
ifdef VERBOSE
	$(if $(AR),$(AR),ar) -rc "$@" $^
	$(if $(RANLIB),$(RANLIB),ranlib) "$@"
else
	@echo "AR $@"
	@$(if $(AR),$(AR),ar) -rc "$@" $^
	@echo "RANLIB $@"
	@$(if $(RANLIB),$(RANLIB),ranlib) "$@"
endif

include $(top_srcdir)/include/mk/generic_leaf_target.mk
