#
#    Copyright (C) 2014, Oracle and/or its affiliates. All Rights Reserved.
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

LIBRELDIR	:= testcases/network/rpc/rpc-tirpc/tests_pack/lib
LIBDIR		:= $(abs_top_builddir)/$(LIBRELDIR)
LIBSRCDIR	:= $(abs_top_srcdir)/$(LIBRELDIR)
LIBRPC-TIRPC	:= $(LIBDIR)/librpc-tirpc.a

$(LIBDIR):
	mkdir -p "$@"

$(LIBRPC-TIRPC): $(LIBDIR)
	$(MAKE) -C $^ -f "$(LIBSRCDIR)/Makefile" all

MAKE_DEPS	+= $(LIBRPC-TIRPC)

LDFLAGS		+= -L$(LIBDIR)
LDLIBS		+= $(TIRPC_LIBS) -lrpc-tirpc
CPPFLAGS	+= $(TIRPC_CPPFLAGS) -I$(LIBSRCDIR) -pthread
