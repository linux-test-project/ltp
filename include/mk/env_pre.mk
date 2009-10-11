#
#    Make pre-include environment Makefile.
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
# Garrett Cooper, September 2009
#
# This Makefile must be included first. NO IF'S, AND'S, OR BUT'S.
#
# This sets the stage for all operations required within Makefiles.
#

ifndef ENV_PRE_LOADED
ENV_PRE_LOADED = 1

# Get the absolute path for the source directory.
top_srcdir			?= $(error You must define top_srcdir before including this file)

# Where's the root source directory?
abs_top_srcdir			:= $(abspath $(top_srcdir))

#
# Where's the root object directory?
#
# Just in case it's not specified, set it to the top srcdir (because the user
# must not have wanted out of build tree support)...
#
top_builddir			?= $(top_srcdir)

# We need the absolute path...
abs_top_builddir		:= $(abspath $(top_builddir))

# Where's the root object directory?
builddir			:= .

abs_builddir			:= $(CURDIR)

# Where's the source located at? Squish all of the / away by using abspath...
abs_srcdir			:= $(abspath $(abs_top_srcdir)/$(subst $(abs_top_builddir),,$(abs_builddir)))

srcdir				:= $(or $(subst $(abs_top_srcdir)/,,$(abs_srcdir)),.)

ifneq ($(abs_builddir),$(abs_srcdir))
export OUT_OF_BUILD_TREE	:= 1
endif

# We can piece together where we're located in the source and object trees with
# just these two vars and $(CURDIR).
export abs_top_srcdir abs_top_builddir

-include $(top_builddir)/include/mk/config.mk

.DEFAULT_GOAL			:= all

endif
