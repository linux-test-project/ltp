#
#   Copyright (c) International Business Machines  Corp., 2004
#
#   This program is free software;  you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY;  without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#   the GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program;  if not, write to the Free Software
#   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#

top_srcdir		?= ../../../..

include	$(top_srcdir)/include/mk/testcases.mk

# dmapi_imp is platform for DMAPI implementation (currently supported: JFS)
dmapi_imp		:= JFS

# dmapi_lib is path of user space library for DMAPI implementation
LDLIBS			+= -ljfsdm

CFLAGS			+= -g -Wall -D$(dmapi_imp)

#	Define USER_SPACE_FAULTS if DMAPI implementation handles user space
#	  faults (JFS has plenty of handle code in user space, which causes
#	  faults when invalid pointers are passed in)
#	Define DIRECTORY_LINKS if DMAPI FS supports links on directories (JFS
#	  does not)
#	Define INTERIOR_HOLES if DMAPI implementation supports probing/punching
#	  interior holes (JFS does); if not defined, only probing/punching
#	  holes to end-of-file is supported
#	Define MULTIPLE_REGIONS if DMAPI implementation supports multiple
#	  interior regions (JFS does); if not defined, only one region
#	  covering entire file is supported
#extra_cflags = -DUSER_SPACE_FAULTS
#extra_cflags = -DDIRECTORY_LINKS
#extra_cflags = -DINTERIOR_HOLES
#extra_cflags = -DMULTIPLE_REGIONS
CFLAGS			+= -DINTERIOR_HOLES -DMULTIPLE_REGIONS

cflags = $(std_cflags) $(extra_cflags)

lflags = $(dmapi_lib)
lflags_thd = $(dmapi_lib) -lpthread

FILTER_OUT_MAKE_TARGETS	:= dm_test

dm_test.o: dm_test.c dm_impl.h

include $(top_srcdir)/include/mk/generic_leaf_target.mk

$(MAKE_TARGETS): %: %.o dm_test.o dm_test.h
