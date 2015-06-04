#
#  Copyright (C) 2012  Red Hat, Inc.
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

MEM_SRCDIR		:= $(abs_top_srcdir)/testcases/kernel/mem
LIBMEM_SRCDIR		:= $(MEM_SRCDIR)/lib

MEM_DIR			:= $(top_builddir)/testcases/kernel/mem
LIBMEM_DIR		:= $(MEM_DIR)/lib
LIBMEM			:= $(LIBMEM_DIR)/libmem.a
FILTER_OUT_DIRS		:= $(LIBMEM_DIR)
CFLAGS			+= -I$(MEM_SRCDIR)/include -pthread
LDLIBS			+= $(NUMA_LIBS) -lmem -lltp
LDFLAGS			+= -L$(LIBMEM_DIR)

$(LIBMEM_DIR):
	mkdir -p "$@"

$(LIBMEM): $(LIBMEM_DIR)
	$(MAKE) -C $^ -f "$(LIBMEM_SRCDIR)/Makefile" all

MAKE_DEPS		+= $(LIBMEM)

trunk-clean:: | lib-clean

lib-clean:: $(LIBMEM_DIR)
	$(MAKE) -C $^ -f "$(LIBMEM_SRCDIR)/Makefile" clean

include $(top_srcdir)/testcases/kernel/include/lib.mk
