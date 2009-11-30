#
#  A Makefile with a collection of reusable functions.
#
#    Copyright (C) 2009, Cisco Systems Inc.
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along
#    with this program; if not, write to the Free Software Foundation, Inc.,
#    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# Garrett Cooper, July 2009
#

SQUOTE			:= '

# ' # to keep colorized editors from going nuts

MAKE_3_80_abspath	= $(shell readlink -f '$(subst $(SQUOTE),\\$(SQUOTE),$(1))')

#
# Generate the directory install dependency separate from generate_install_rule
# to avoid noise from Make about redefining existing targets, for good reason.
#
# 1 -> The base install directory, based off $(prefix), or another
#      $(prefix)-like variable. Automatically gets $(DESTDIR) tacked on it.
# 2 -> The install target. This can be relative to $(srcdir), but will most
#      likely be `empty'.
#
define generate_install_rule_dir_dep
ifdef MAKE_3_80_COMPAT
DIR	:= $$(call MAKE_3_80_abspath,$(DESTDIR)/$(1)/$(2))
else
DIR	:= $$(abspath $(DESTDIR)/$(1)/$(2))
endif
$$(DIR):
	mkdir -p "$$@"
endef

#
# Generate an install rule which also creates the install directory if needed
# to avoid unnecessary bourne shell based for-loops and install errors, as well
# as adhoc install rules.
#
# 1 -> Target basename.
# 2 -> Source directory.
# 3 -> Destination directory.
#
define generate_install_rule

# This doesn't do Jack currently, as per the $(MAKECMDGOALS) check in
# env_post.mk. I can revisit this `enhancement' later.
#CLEAN_TARGETS		+= $$(INSTALL_FILE)

ifdef MAKE_3_80_COMPAT
INSTALL_FILES		+= $$(call MAKE_3_80_abspath,$(DESTDIR)/$(3)/$(1))
else
INSTALL_FILES		+= $$(abspath $(DESTDIR)/$(3)/$(1))
endif # MAKE_3_80_COMPAT

ifdef MAKE_3_80_COMPAT
$$(call MAKE_3_80_abspath,$(DESTDIR)/$(3)/$(1)): \
    $$(call MAKE_3_80_abspath,$$(dir $(DESTDIR)/$(3)/$(1)))
else
$$(abspath $(DESTDIR)/$(3)/$(1)): \
    $$(abspath $$(dir $(DESTDIR)/$(3)/$(1)))
endif # MAKE_3_80_COMPAT
ifdef INSTALL_PRE
	@echo "Executing preinstall command."
	$$(INSTALL_PRE)
endif
	install -m $$(INSTALL_MODE) "$(2)/$(1)" "$$@"
ifdef INSTALL_POST
	@echo "Executing preinstall command."
	$$(INSTALL_POST)
endif
endef

#
# Create a vpath rule for a given extension.
#
# 1 -> file extension
# 2 -> search directory. Defaults to $(abs_srcdir) if not specified.
# 
define generate_vpath_rule
ifdef MAKE_3_80_COMPAT
ifeq ($$(strip $(2)),)
vpath %.$(1)	$(abs_srcdir)
else
vpath %.$(1)	$(2)
endif # End $$(strip $(2))
else
vpath %.$(1)	$$(if $(2),$(2),$(abs_srcdir))
endif # End ifdef MAKE_3_80_COMPAT
endef

#
# Set SUBDIRS to the subdirectories where Makefiles were found.
# 
define get_make_dirs
SUBDIRS	?= $$(subst $(abs_srcdir)/,,$$(patsubst %/Makefile,%,$$(wildcard $(abs_srcdir)/*/Makefile)))
SUBDIRS	:= $$(filter-out $$(FILTER_OUT_DIRS),$$(SUBDIRS))
endef
