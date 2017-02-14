#
#    kernel/fs/doio testcases Makefile.
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

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

CFLAGS			+= -D_LARGEFILE64_SOURCE -Wall \
			   -I$(abs_top_srcdir)/testcases/kernel/fs/doio/include/
LDLIBS			+= -lrt -lpthread

MAKE_TARGETS		:= growfiles doio iogen
INSTALL_TARGETS		:= rwtest


$(MAKE_TARGETS): dataascii.o databin.o file_lock.o forker.o open_flags.o \
		 datapid.o write_log.o pattern.o string_to_tokens.o \
		 bytes_by_prefix.o

%.o: $(abs_top_srcdir)/testcases/kernel/fs/doio/%.c

include $(top_srcdir)/include/mk/generic_leaf_target.mk
