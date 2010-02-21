#
#    kernel/mem/hugetlb testcase suite common definitions Makefile.
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

# DO NOT USE THIS FILE FOR hugetlb / lib!!!
LIBIPCDIR		:= ../lib
LIBIPC			:= $(LIBIPCDIR)/libipc_hugetlb.a
LIBMMDIR		:= ../../libmm
LIBMM			:= $(LIBMMDIR)/libmm.a

$(LIBIPCDIR) $(LIBMMDIR): %:
	mkdir -p "$@"

$(LIBMM): $(LIBMMDIR)
	$(MAKE) -C "$^" -f "$(abs_srcdir)/$^/Makefile" all

$(LIBIPC): $(LIBIPCDIR)
	$(MAKE) -C "$^" -f "$(abs_srcdir)/$^/Makefile" all

CPPFLAGS		+= -I$(abs_srcdir)/$(LIBMMDIR) -I$(abs_srcdir)/$(LIBIPCDIR)

LDFLAGS			+= -L$(abs_builddir)/$(LIBMMDIR) -L$(abs_builddir)/$(LIBIPCDIR)

LDLIBS			+= -lmm -lipc_hugetlb

MAKE_DEPS		:= $(LIBMM) $(LIBIPC)

# vim: syntax=make
