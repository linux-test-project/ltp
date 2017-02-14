#
#    gitignore generation include Makefile.
#
#    Copyright (C) 2011, Linux Test Project.
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
# Ngie Cooper, January 2011
#

CLEAN_TARGETS+=		gitignore_clean

BEFORE:=		.gitignore-before

AFTER:=			.gitignore-after

IGNORE_DIR_EXPR:=	egrep -v "^$$(echo "$(AUTOCONFED_SUBDIRS)" | tr " " "|")"

# NOTE: The underscore is used in place of a dash to avoid implicit rule
# evaluation in top-level Makefile.
.PHONY: gitignore_clean
gitignore_clean:
	$(RM) -f $(BEFORE) $(AFTER)

$(BEFORE):
	$(MAKE) distclean
	$(MAKE) ac-maintainer-clean
	find . | $(IGNORE_DIR_EXPR) > $@

$(AFTER):
	$(MAKE) autotools
	./configure --prefix=/dev/null
	$(MAKE) all
	find . | $(IGNORE_DIR_EXPR) > $@
	# Set everything in autoconf land back to a sane state.
	$(MAKE) distclean

.gitignore: | $(BEFORE) $(AFTER)
	diff -u $(BEFORE) $(AFTER) | grep '^+' | sed -e 's,^\+,,g' > $@
