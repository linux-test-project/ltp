# SPDX-License-Identifier: GPL-2.0-or-later
# Make pre-include environment Makefile.
# Copyright (c) Linux Test Project, 2009-2020
# Copyright (c) Cisco Systems Inc., 2009
# Ngie Cooper, September 2009
#
# This Makefile must be included first. NO IF'S, AND'S, OR BUT'S.
# This sets the stage for all operations required within Makefiles.

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

# Get the absolute path for the source directory.
top_srcdir			?= $(error You must define top_srcdir before including this file)

include $(top_srcdir)/include/mk/functions.mk

# Where's the root source directory?
abs_top_srcdir			:= $(abspath $(top_srcdir))

#
# Where's the root object directory?
#
# Just in case it's not specified, set it to the top srcdir (because the user
# must not have wanted out of build tree support)...
#
top_builddir			?= $(top_srcdir)

# We need the absolute path
abs_top_builddir		:= $(abspath $(top_builddir))

# Where's the root object directory?
builddir			:= .

abs_builddir			:= $(CURDIR)

cwd_rel1			:= $(subst $(abs_top_builddir),,$(abs_builddir))
cwd_rel2			:= $(subst $(abs_top_builddir)/,,$(abs_builddir))
cwd_rel_from_top		:= $(if $(cwd_rel1),$(cwd_rel2),$(cwd_rel1))

# Where's the source located at? Squish all of the / away by using abspath
abs_srcdir			:= $(abspath $(abs_top_srcdir)/$(cwd_rel_from_top))

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

.DEFAULT_GOAL			:= all

endif	# END autotools, *clean...

BUILD_TREE_STATE		?= $(BUILD_TREE_UNCONFIGURED)

# We can piece together where we're located in the source and object trees with
# just these two vars and $(CURDIR).
export abs_top_srcdir abs_top_builddir BUILD_TREE_STATE

ifeq ($V,1)
VERBOSE=1
endif

endif
