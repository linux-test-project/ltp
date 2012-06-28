#
#  Copyright (C) 2012 Linux Test Project, Inc.
#
#  This program is free software;  you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY;  without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#  the GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program;  if not, write to the Free Software
#  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#

CPPFLAGS		+= $(NUMA_CPPFLAGS) -I../numa/lib/
LIBDIR			:= ../numa/lib
LIB			:= $(LIBDIR)/libnuma_helper.a
MAKE_DEPS		+= $(LIB)
LDLIBS			+= $(NUMA_LIBS) -lnuma_helper -lltp
LDFLAGS			+= -L$(LIBDIR)

$(LIB): $(LIBDIR)
		$(MAKE) -C $^ -f "$(abs_srcdir)/$^/Makefile" all
