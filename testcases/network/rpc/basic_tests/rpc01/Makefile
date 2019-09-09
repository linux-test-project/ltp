#
#    network/rpc/basic_tests/rpc01 test suite Makefile.
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

top_srcdir		?= ../../../../..

include $(top_srcdir)/include/mk/env_pre.mk

INSTALL_TARGETS		:= rpc01.sh
SUBDIRS			:= datafiles

CPPFLAGS		+= -I$(abs_srcdir)/lib
LDFLAGS			+= -L$(abs_builddir)/lib
LDLIBS			+= -lrpc01

LIBDIR			:= lib
LIB			:= $(LIBDIR)/librpc01.a

$(LIBDIR):
	mkdir -p "$@"

$(LIB): $(LIBDIR)
	$(MAKE) -C $^ -f "$(abs_srcdir)/$^/Makefile" all

trunk-clean:: | lib-clean

lib-clean:: $(LIBDIR)
	$(MAKE) -C $^ -f "$(abs_srcdir)/$^/Makefile" clean

MAKE_TARGETS		:= rpc1 rpc_server
MAKE_DEPS		:= $(LIB)

include $(top_srcdir)/include/mk/generic_trunk_target.mk
include $(top_srcdir)/testcases/network/rpc/rpc-tirpc/tests_pack/Makefile.inc
