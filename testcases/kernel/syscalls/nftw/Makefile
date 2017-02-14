#
#    testcases/kernel/syscalls Makefile.
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

CPPFLAGS		+= -D_XOPEN_SOURCE=500 -D_XOPEN_SOURCE_EXTENDED

MAKE_TARGETS		:= nftw01 nftw6401

nftw01: CPPFLAGS += -D_LARGEFILE_SOURCE

nftw01: nftw.o tools.o test_func.o test.o lib.o 
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) $(OUTPUT_OPTION)

nftw64.o tools64.o test_func64.o test64.o lib64.o: %.o: %.c

%64.o: CPPFLAGS += -D_LARGEFILE64_SOURCE

nftw6401: nftw64.o tools64.o test_func64.o test64.o lib64.o
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) $(OUTPUT_OPTION)

include $(top_srcdir)/include/mk/generic_leaf_target.mk
