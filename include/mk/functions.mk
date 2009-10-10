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

#$$(warning Called generate_install_rule_dir_dep for $(abspath $(DESTDIR)/$(1)/$(2)) )

$(abspath $(DESTDIR)/$(1)/$(2)):
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
#CLEAN_TARGETS		+= $(DESTDIR)/$(3)/$(1)
INSTALL_FILES		+= $(DESTDIR)/$(3)/$(1)

# $$(warning Called generate_install_rule for $(2)/$(1) -> $(DESTDIR)/$(3)/$(1) )

# XXX (garrcoop): FIXME -- relative path install based logic doesn't work
# 100%, as proven by `testcases/network/tcp_cmds/generate'. This needs to be
# seriously fixed.
$(DESTDIR)/$(3)/$(1): $$(abspath $(DESTDIR)/$(3))
ifdef INSTALL_PRE
	@echo "Executing preinstall command."
	$$(INSTALL_PRE)
endif
	test -d "$$(@D)" || mkdir -p "$$(@D)"
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
vpath %.$(1)	$$(if $(2),$(2),$(abs_srcdir))
endef

#
# Set SUBDIRS to the subdirectories where Makefiles were found.
# 
define get_make_dirs
SUBDIRS	?= $$(subst $(abs_srcdir)/,,$$(patsubst %/Makefile,%,$$(wildcard $(abs_srcdir)/*/Makefile)))
SUBDIRS	:= $$(filter-out $$(FILTER_OUT_DIRS),$$(SUBDIRS))
endef
