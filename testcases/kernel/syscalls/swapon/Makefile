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

NEEDSPECIAL	:= $(shell echo MAX_SWAPFILES | $(CC) -E -xc -include linux/swap.h 2>/dev/null - | tail -n 1 | grep 32; echo $?)
ifneq ($(strip $(NEEDSPECIAL)),)
export CFLAGS	+= -DOLDER_DISTRO_RELEASE
endif

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

FILTER_OUT_MAKE_TARGETS         := libswapon

include $(top_srcdir)/include/mk/generic_leaf_target.mk

$(MAKE_TARGETS): %: %.o libswapon.o
