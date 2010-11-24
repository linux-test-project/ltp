#
#    lib Makefile.
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

top_srcdir		?= ..

include $(top_srcdir)/include/mk/env_pre.mk

ifeq ($(UCLINUX),1)
CFLAGS			+= -D_USC_LIB_
endif

#CPPFLAGS		+= -DGARRETT_IS_A_PEDANTIC_BASTARD

FILTER_OUT_LIBSRCS	:= mount_utils.c

LIB			:= libltp.a

pc_file			:= $(DESTDIR)/$(datarootdir)/pkgconfig/ltp.pc

INSTALL_TARGETS		:= $(pc_file)

$(pc_file):
	test -d "$(@D)" || mkdir -p "$(@D)"
	install -m $(INSTALL_MODE) "$(builddir)/$(@F)" "$@"

include $(top_srcdir)/include/mk/lib.mk
