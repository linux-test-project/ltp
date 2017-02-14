#
#    kernel/controllers/cpuctl test suite Makefile.
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
# Ngie Cooper, July 2009
#

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk
include $(abs_srcdir)/../Makefile.inc

INSTALL_TARGETS		:= run_cpuctl_test.sh run_cpuctl_stress_test.sh parameters.sh run_cpuctl_latency_test.sh

LDLIBS			+= -lm -lcontrollers

include $(top_srcdir)/include/mk/generic_leaf_target.mk
