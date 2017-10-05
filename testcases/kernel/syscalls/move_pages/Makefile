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
#  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

CPPFLAGS		+= -I$(abs_srcdir)/../utils

INSTALL_TARGETS		:= move_pages.sh

MAKE_TARGETS		:= $(patsubst $(abs_srcdir)/%.c,%,$(wildcard $(abs_srcdir)/*[0-9].c))

$(MAKE_TARGETS): %: %.o move_pages_support.o

LDLIBS			+= -lpthread -lrt

include $(top_srcdir)/testcases/kernel/include/lib.mk
include $(top_srcdir)/include/mk/generic_leaf_target.mk
