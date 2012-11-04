#
#  Copyright (c) International Business Machines  Corp., 2001
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
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#

top_srcdir			?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

CFLAGS				+= -DSHARED_OFILE -D_GNU_SOURCE

LDLIBS				+= -lpthread

FILTER_OUT_MAKE_TARGETS		:= diotest_routines

INSTALL_TARGETS			:= test_dma_thread_diotest7.sh

include $(top_srcdir)/include/mk/generic_leaf_target.mk

$(MAKE_TARGETS): %: %.o diotest_routines.o
