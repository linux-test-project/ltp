#
#    network/tcp_cmds/sendfile testcases Makefile.
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
# Ngie Cooper, September 2009
#

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk
include $(abs_srcdir)/../Makefile.inc

CPPFLAGS		+= -I$(abs_srcdir)/../include

INSTALL_TARGETS		:= sendfile01.sh

MAKE_TARGETS		+= testsf_c testsf_s testsf_c6 testsf_s6

testsf_c6.o testsf_s6.o: CPPFLAGS += -DINET6

testsf_c6.o testsf_s6.o: %6.o: %.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

include $(top_srcdir)/include/mk/generic_leaf_target.mk
