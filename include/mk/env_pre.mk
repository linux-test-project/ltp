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
# Ngie Cooper, September 2009
#
# This Makefile must be included first. NO IF'S, AND'S, OR BUT'S.
#
# This sets the stage for all operations required within Makefiles.
#

ifndef ENV_PRE_LOADED
ENV_PRE_LOADED = 1

# "out-of-build-tree" build.
BUILD_TREE_BUILDDIR_INSTALL	:= 1
# "in-srcdir" build / install.
BUILD_TREE_SRCDIR_INSTALL	:= 2
# "in-srcdir" build, non-srcdir install.
BUILD_TREE_NONSRCDIR_INSTALL	:= 3
# configure not run.
BUILD_TREE_UNCONFIGURED		:= 4

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

# If config.mk or features.mk doesn't exist it's not an error for some targets
# which are filtered below (e.g. clean). However these config files may be
# needed for those targets (eg. the open posix testsuite is not cleaned even if
# it's enabled by configure) thus it would be wise to do silent inclusion.
ifneq ("$(wildcard $(abs_top_builddir)/include/mk/config.mk)","")
include $(abs_top_builddir)/include/mk/config.mk
endif
ifneq ("$(wildcard $(abs_top_builddir)/include/mk/features.mk)","")
include $(abs_top_builddir)/include/mk/features.mk
endif

# autotools, *clean, and help don't require config.mk, features.mk, etc...
ifeq ($(filter autotools %clean .gitignore gitignore.% help,$(MAKECMDGOALS)),)

include $(abs_top_builddir)/include/mk/config.mk
include $(abs_top_builddir)/include/mk/features.mk

# START out-of-build-tree check.
ifneq ($(abs_builddir),$(abs_srcdir))
BUILD_TREE_STATE		:= $(BUILD_TREE_BUILDDIR_INSTALL)
else
# Else, not out of build tree..

# START srcdir build-tree install checks
ifeq ($(strip $(DESTDIR)$(prefix)),)
BUILD_TREE_STATE		:= $(BUILD_TREE_SRCDIR_INSTALL)
else  # Empty $(DESTDIR)$(prefix)
ifeq ($(abs_top_srcdir),$(prefix))
BUILD_TREE_STATE		:= $(BUILD_TREE_SRCDIR_INSTALL)
endif
# END srcdir build-tree install checks
endif
# END out-of-build-tree check.
endif

# Is the build-tree configured yet?
ifeq ($(BUILD_TREE_STATE),)
ifneq ($(wildcard $(abs_top_builddir)/include/mk/config.mk),)
BUILD_TREE_STATE		:= $(BUILD_TREE_NONSRCDIR_INSTALL)
endif
endif

ifeq ($(MAKE_3_80_COMPAT),1)
# Trick make 3.80 into thinking that the default goal is all.
.PHONY: default
default: all
else
.DEFAULT_GOAL			:= all
endif

endif	# END autotools, *clean...

BUILD_TREE_STATE		?= $(BUILD_TREE_UNCONFIGURED)

# We can piece together where we're located in the source and object trees with
# just these two vars and $(CURDIR).
export abs_top_srcdir abs_top_builddir BUILD_TREE_STATE

endif
