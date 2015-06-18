#
#  Copyright (c) 2015 Cyril Hrubis <chrubis@suse.cz>
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

futex_wait02: LDLIBS+=-lrt
futex_wake03: LDLIBS+=-lrt
futex_wait03: CFLAGS+=-pthread
futex_wake02: CFLAGS+=-pthread
futex_wake04: CFLAGS+=-pthread
futex_wait05: LDLIBS+=-lrt
futex_wait_bitset01: LDLIBS+=-lrt
futex_wait_bitset02: LDLIBS+=-lrt

include $(top_srcdir)/include/mk/testcases.mk
include $(top_srcdir)/include/mk/generic_leaf_target.mk
