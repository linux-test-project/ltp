# Copyright (c) International Business Machines  Corp., 2008
# Copyright (c) 2015 Red Hat, Inc.
#
# This program is free software;  you can redistribute it and#or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Author:      Veerendra <veeren@linux.vnet.ibm.com>
################################################################################

top_srcdir              ?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk
include $(abs_srcdir)/../Makefile.inc

LDLIBS                  := -lclone -lltp

include $(top_srcdir)/include/mk/generic_leaf_target.mk
