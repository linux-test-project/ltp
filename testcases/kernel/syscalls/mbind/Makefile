#
#  Copyright (c) International Business Machines  Corp., 2009
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

#
# gcc complains about unused functions / variables when HAS_NUMA_H isn't
# defined, so let's just skip the warning. It's nothing more than a few extra
# variables and functions allocated on the stack, so meh... no harm, no foul.
#
ifeq ($(NUMA_LIBS),)
CPPFLAGS		+= -Wno-unused
endif

CPPFLAGS		+= -I$(abs_srcdir)/../utils/

include $(top_srcdir)/testcases/kernel/include/lib.mk
include $(top_srcdir)/include/mk/generic_leaf_target.mk
