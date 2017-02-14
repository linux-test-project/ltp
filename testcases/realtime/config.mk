#
#    Default include definitions for realtime test suite Makefile.
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
# Ngie Cooper, September 2009
#


# Default stuff common to all testcases

realtime_reldir		:= testcases/realtime

realtime_builddir	:= $(abs_top_builddir)/$(realtime_reldir)
realtime_srcdir		:= $(abs_top_srcdir)/$(realtime_reldir)

CPPFLAGS		+= -I$(realtime_srcdir)/include
CPPFLAGS		+= -I$(realtime_builddir)/include
CFLAGS			+= -D_GNU_SOURCE
LDLIBS			+= -lrealtime -lpthread -lrt -lm
LDFLAGS			+= -L$(realtime_builddir)/lib

INSTALL_DIR		:= $(srcdir)
