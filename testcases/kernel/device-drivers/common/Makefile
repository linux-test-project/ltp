# Copyright (c) 2019 Texas Instruments Incorporated
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it would be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write the Free Software Foundation,
# Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

INSTALL_DIR		:= testcases/bin/ddt/$(notdir $(CURDIR:%/=%))

FILTER_OUT_DIRS :=

# We don't want to copy over the Makefile
UNWANTED_FILES	:= Makefile

INSTALL_MODE	:= 00755

INSTALL_TARGETS	:= $(filter-out $(UNWANTED_FILES),$(notdir $(patsubst $(abs_srcdir)/%,%,$(wildcard $(abs_srcdir)/*))))

MAKE_TARGETS	:=

include $(top_srcdir)/include/mk/generic_leaf_target.mk
