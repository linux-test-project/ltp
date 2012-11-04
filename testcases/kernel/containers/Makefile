#
# Copyright (c) International Business Machines  Corp., 2007
#
# This program is free software;  you can redistribute it and#or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program;  if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#

top_srcdir		?= ../../..

include $(top_srcdir)/include/mk/env_pre.mk

LIBDIR			:= libclone

FILTER_OUT_DIRS		:= $(LIBDIR)

LIB			:= $(LIBDIR)/libclone.a

LDLIBS			:= -ldl -lltp

INSTALL_TARGETS		:= *.sh

$(LIBDIR):
	mkdir -p "$@"

# Make the target the real lib so we don't have to deal with rebuilding this
# every time the dependency is evaluated, like with PHONY rules.
$(LIB): $(LIBDIR)
	$(MAKE) -C $^ -f "$(abs_srcdir)/$(LIBDIR)/Makefile" all

MAKE_DEPS		:= $(LIB)

trunk-clean:: | lib-clean

lib-clean:: $(LIBDIR)
	$(MAKE) -C $^ -f "$(abs_srcdir)/$(LIBDIR)/Makefile" clean

include $(top_srcdir)/include/mk/generic_trunk_target.mk
