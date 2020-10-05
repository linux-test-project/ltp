#
#  A Makefile with a collection of reusable functions.
#
#    Copyright (c) Linux Test Project, 2009-2020
#    Copyright (c) Cisco Systems Inc., 2009
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
