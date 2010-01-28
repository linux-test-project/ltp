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

ifndef MAKE_VERSION_CHECK
export MAKE_VERSION_CHECK = 1
ifneq ($(firstword $(sort 3.80 $(MAKE_VERSION))),3.80)
$(error Your version of make $(MAKE_VERSION) is too old. Upgrade to at least 3.80; 3.81+ is preferred)
else
ifneq ($(filter 3.80%,$(MAKE_VERSION)),)
export MAKE_3_80_COMPAT	:= 1
endif # make 3.80?
endif # At least make 3.80?
endif # MAKE_VERSION_CHECK

# Get the absolute path for the source directory.
top_srcdir			?= $(error You must define top_srcdir before including this file)

include $(top_srcdir)/include/mk/functions.mk

# Where's the root source directory?
ifdef MAKE_3_80_COMPAT
abs_top_srcdir			:= $(call MAKE_3_80_abspath,$(top_srcdir))
else
abs_top_srcdir			:= $(abspath $(top_srcdir))
endif

#
# Where's the root object directory?
#
# Just in case it's not specified, set it to the top srcdir (because the user
# must not have wanted out of build tree support)...
#
top_builddir			?= $(top_srcdir)

# We need the absolute path...
ifdef MAKE_3_80_COMPAT
abs_top_builddir		:= $(call MAKE_3_80_abspath,$(top_builddir))
else
abs_top_builddir		:= $(abspath $(top_builddir))
endif

# Where's the root object directory?
builddir			:= .

abs_builddir			:= $(CURDIR)

cwd_rel_from_top		:= $(subst $(abs_top_builddir),,$(abs_builddir))

# Where's the source located at? Squish all of the / away by using abspath...
ifdef MAKE_3_80_COMPAT
abs_srcdir			:= $(call MAKE_3_80_abspath,$(abs_top_srcdir)/$(cwd_rel_from_top))
else
abs_srcdir			:= $(abspath $(abs_top_srcdir)/$(cwd_rel_from_top))
endif

srcdir				:= $(strip $(subst $(abs_top_srcdir)/,,$(abs_srcdir)))

ifeq ($(srcdir),)
srcdir				:= .
endif

# We can piece together where we're located in the source and object trees with
# just these two vars and $(CURDIR).
export abs_top_srcdir abs_top_builddir

# DO NOT MOVE THIS BELOW include [..]/config.mk (will break out-of-build tree
# checks)!
ifneq ($(abs_builddir),$(abs_srcdir))
OUT_OF_BUILD_TREE		:= 1
else
# Stub support for installing directly in the build tree; the support is not
# there yet, but the variable itself has its own uses...
ifeq ($(strip $(DESTDIR)$(prefix)),)
INSTALL_IN_BUILD_TREE		:= 1
else
ifeq ($(subst $(abs_top_srcdir),,$(prefix)),)
INSTALL_IN_BUILD_TREE		:= 1
endif
endif
endif

ifeq ($(filter autotools %clean help,$(MAKECMDGOALS)),)
include $(abs_top_builddir)/include/mk/config.mk
endif

# make 3.80 called it .DEFAULT_TARGET.
.DEFAULT_GOAL			:= all

endif
