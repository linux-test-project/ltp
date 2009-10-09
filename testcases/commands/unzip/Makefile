#
#    commands/unzip testcases Makefile.
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

top_srcdir		?= ../../..

include $(top_srcdir)/include/mk/env_pre.mk

# Only build and install if we have the zip command.
HAVE_ZIPTOOLS		:= $(shell if which zip >/dev/null; then echo yes; else echo no; fi)

DIR			:= tst_unzip.dir

MAKE_TARGETS		:=

ifeq ($(HAVE_ZIPTOOLS),yes)
INSTALL_TARGETS		:= unzip_tests.sh
$(INSTALL_TARGETS): $(ZIPFILE)
endif

$(ZIPFILE): $(DIR)
	$(ZIP) -r $@ $(DIR) && chmod 777 $@

CLEAN_TARGETS		:= $(DIR)

# This target should be auto-removed by Make
.INTERMEDIATE: $(DIR)
$(DIR): unzip_genfile.sh
	./$^ $(DIR)

include $(top_srcdir)/include/mk/generic_leaf_target.mk
