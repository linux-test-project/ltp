#
#    network/tcp_cmds/perf_lan testcases Makefile.
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
# Garrett Cooper, July 2009
#

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

CPPFLAGS		+= -D_GNU_SOURCE -I$(abs_srcdir)/../include

# XXX (garrcoop): alias casting problems.
CPPFLAGS		+= -Wno-error

INSTALL_TARGETS		:= perf_lan

# NOTE (garrcoop): Once everything compiles with the same sources using
# pingpong.c, then the following commented code should be uncommented and
# pingpong6.c should be nuked. Unfortunately that isn't quite yet...
#
#MAKE_TARGETS		:= pingpong pingpong6
#
#%6: CPPFLAGS += -DINET6
#
#%6.o: %.c
#	$(COMPILE.c) $(OUTPUT_OPTION) $<

include $(top_srcdir)/include/mk/generic_leaf_target.mk
