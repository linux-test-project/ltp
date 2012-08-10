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

KERNEL_DIR		:= $(top_srcdir)/testcases/kernel
LIBKERNTEST_DIR		:= $(KERNEL_DIR)/lib
LIBKERNTEST		:= $(KERNEL_DIR)/libkerntest.a
CPPFLAGS		+= $(NUMA_CPPFLAGS) -I$(KERNEL_DIR)/include
LDLIBS			+= $(NUMA_LIBS) -lkerntest -lltp
LDFLAGS			+= -L$(LIBKERNTEST_DIR)

$(LIBKERNTEST_DIR):
	mkdir -p "$@"

$(LIBKERNTEST): $(LIBKERNTEST_DIR)
	$(MAKE) -C $^ -f "$(abs_srcdir)/$^/Makefile" all

MAKE_DEPS		+= $(LIBKERNTEST)

trunk-clean:: | lib-clean

lib-clean:: $(LIBKERNTEST_DIR)
	$(MAKE) -C $^ -f "$(abs_srcdir)/$^/Makefile" clean
