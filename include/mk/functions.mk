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
# Ngie Cooper, July 2009
#

SQUOTE			:= '

# ' # to keep colorized editors from going nuts

MAKE_3_80_realpath	= $(shell $(top_srcdir)/scripts/realpath.sh '$(subst $(SQUOTE),\\$(SQUOTE),$(1))')

MAKE_3_80_abspath	= $(shell $(top_srcdir)/scripts/abspath.sh '$(subst $(SQUOTE),\\$(SQUOTE),$(1))')

#
# NOTE (garrcoop):
#
# The following functions are (sometimes) split into 3.80 and 3.81+
# counterparts, and not conditionalized inside of the define(s) to work around
# an issue with how make 3.80 evaluates defines.
#
# SO DO NOT INTERNALIZE CONDITIONALS IN DEFINES OR YOU WILL BREAK MAKE 3.80!
#

#
# Generate an install rule which also creates the install directory if needed
# to avoid unnecessary bourne shell based for-loops and install errors, as well
# as adhoc install rules.
#
# 1 -> Target basename.
# 2 -> Source directory.
# 3 -> Destination directory.
#
ifdef MAKE_3_80_COMPAT
define generate_install_rule

INSTALL_FILES		+= $$(call MAKE_3_80_abspath,$$(DESTDIR)/$(3)/$(1))

$$(call MAKE_3_80_abspath,$$(DESTDIR)/$(3)/$(1)): \
    $$(call MAKE_3_80_abspath,$$(dir $$(DESTDIR)/$(3)/$(1)))
	install -m $$(INSTALL_MODE) "$(2)/$(1)" "$$@"
endef
else # not MAKE_3_80_COMPAT
define generate_install_rule

INSTALL_FILES		+= $$(abspath $$(DESTDIR)/$(3)/$(1))

$$(abspath $$(DESTDIR)/$(3)/$(1)): \
    $$(abspath $$(dir $$(DESTDIR)/$(3)/$(1)))
	install -m $$(INSTALL_MODE) "$(2)/$(1)" "$$@"
endef
endif # END MAKE_3_80_COMPAT

#
# Set SUBDIRS to the subdirectories where Makefiles were found.
#
define get_make_dirs
SUBDIRS	?= $$(subst $$(abs_srcdir)/,,$$(patsubst %/Makefile,%,$$(wildcard $$(abs_srcdir)/*/Makefile)))
SUBDIRS	:= $$(filter-out $$(FILTER_OUT_DIRS),$$(SUBDIRS))
endef
