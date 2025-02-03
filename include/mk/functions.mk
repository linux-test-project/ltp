# SPDX-License-Identifier: GPL-2.0-or-later
# A Makefile with a collection of reusable functions.
# Copyright (c) Linux Test Project, 2009-2020
# Copyright (c) Cisco Systems Inc., 2009
# Ngie Cooper, July 2009

# Generate an install rule which also creates the install directory if needed
# to avoid unnecessary bourne shell based for-loops and install errors, as well
# as adhoc install rules.
#
# 1 -> Target basename.
# 2 -> Source directory.
# 3 -> Destination directory.

define generate_install_rule

INSTALL_FILES		+= $$(abspath $$(DESTDIR)/$(3)/$(1))

$$(abspath $$(DESTDIR)/$(3)/$(1)): \
    $$(abspath $$(dir $$(DESTDIR)/$(3)/$(1)))
	install -m $$(INSTALL_MODE) $(shell test -d "$(2)/$(1)" && echo "-d") $(PARAM) "$(2)/$(1)" $$@
	$(shell test -d "$(2)/$(1)" && echo "install -m "'$$(INSTALL_MODE) $(PARAM)' "$(2)/$(1)/*" -t '$$@')
endef

#
# Set SUBDIRS to the subdirectories where Makefiles were found.
#
define get_make_dirs
SUBDIRS	?= $$(subst $$(abs_srcdir)/,,$$(patsubst %/Makefile,%,$$(wildcard $$(abs_srcdir)/*/Makefile)))
SUBDIRS	:= $$(filter-out $$(FILTER_OUT_DIRS),$$(SUBDIRS))
endef
