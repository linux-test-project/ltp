#
#    Top-level Makefile for LTP. See INSTALL for more info.
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

# Force IDcheck.sh to fix any issues found with $(DESTDIR)/etc/group and
# $(DESTDIR)/etc/passwd automatically when after running the top-level
# install target.
CREATE_ENTRIES		?= 0

top_srcdir		?= $(CURDIR)

include $(top_srcdir)/include/mk/env_pre.mk
include $(top_srcdir)/include/mk/automake.mk

# This variable is automatically changed from help to all once someone has
# run configure, or the equivalent steps manually, as described in
# README.mk-user.
ifeq ($(wildcard $(top_builddir)/include/config.h),)
.DEFAULT_GOAL		:= help
else
.DEFAULT_GOAL		:= all
endif

.SUFFIXES:
.SUFFIXES: .am .default .h .in .m4 .mk

vpath %.am		$(top_srcdir)/m4
vpath %.default		$(top_srcdir)/include
vpath %.h		$(top_srcdir)/include:$(top_builddir)/include
vpath %.in		$(top_srcdir)/include
vpath %.m4		$(top_srcdir)/m4
vpath %.mk		$(top_srcdir)/mk:$(top_srcdir)/mk/include

# Skip running IDcheck.sh at the end of install?
SKIP_IDCHECK		?= 0

# User wants uclinux binaries?
UCLINUX			?= 0

# User wants uclinux binaries. Do this for backwards compatibility, but
# print out a warning.
ifneq ($(filter uclinux%,$(MAKECMDGOALS)),)
$(warning uclinux* targets will be deprecated in the future.)
$(warning Please specify UCLINUX=1 on the command line to achieve the same)
$(warning functionality.)
UCLINUX			:= 1
endif

export UCLINUX

# CLEAN_TARGETS:	Targets which exist solely in clean.
# COMMON_TARGETS:	Targets which exist in all, clean, and install.
# INSTALL_TARGETS:	Targets which exist in clean and install (contains
# 			COMMON_TARGETS).
# BOOTSTRAP_TARGETS:	Directories required to bootstrap out-of-build-tree
# 			support.

# We're not using uclinux based targets (default).
ifneq ($(UCLINUX),1)
COMMON_TARGETS		:= pan utils
INSTALL_TARGETS		:= doc
endif

COMMON_TARGETS		+= testcases tools
INSTALL_TARGETS		+= $(COMMON_TARGETS) runtest
CLEAN_TARGETS		+= $(COMMON_TARGETS) lib include runtest
BOOTSTRAP_TARGETS	:= $(sort $(COMMON_TARGETS) $(CLEAN_TARGETS)\
				  $(INSTALL_TARGETS))

CLEAN_TARGETS		:= $(addsuffix -clean,$(CLEAN_TARGETS))
INSTALL_TARGETS		:= $(addsuffix -install,$(INSTALL_TARGETS))
MAKE_TARGETS		:= $(addsuffix -all,$(filter-out lib,$(COMMON_TARGETS)))

# There's no reason why we should run `all' twice. Otherwise we're just wasting
# 3+ mins of useful CPU cycles on a modern machine, and even more time on an
# overtaxed one, or one where -j => 1 was specified.
all: $(addsuffix -all,$(COMMON_TARGETS))

$(MAKE_TARGETS): lib-install

.PHONY: include-all include-install
include-install: include/config.h include/mk/config.mk include-all

INSTALL_DIR		:= $(DESTDIR)/$(prefix)

# build tree bootstrap targets and $(INSTALL_DIR) target.
$(addprefix $(abs_top_builddir)/,$(BOOTSTRAP_TARGETS)) $(INSTALL_DIR):
	mkdir -m 00755 -p "$@"

## Pattern based subtarget rules.
lib-all: include-install

lib-install: lib-all

.SECONDEXPANSION:
$(MAKE_TARGETS) include-all lib-all: %-all: $(abs_top_builddir)/$$*
	$(MAKE) -C $* -f "$(abs_top_srcdir)/$*/Makefile" all 

# Let's not conflict with ac-clean, maintainer-clean, etc, so.
.SECONDEXPANSION:
$(filter-out include-clean,$(CLEAN_TARGETS)):: %-clean: $(abs_top_builddir)/$$*
	-$(MAKE) -C $* -f "$(abs_top_srcdir)/$*/Makefile" clean

# Just like everything depends on include-all / -install, we need to get rid
# of include last to ensure that things won't be monkey screwed up. Only do
# this if we're invoking clean or a subclean directly though.
ifneq ($(filter clean,$(MAKECMDGOALS)),)
INCLUDE_CLEAN_RDEP_SUBJECT	:= $(CLEAN_TARGETS)
else
ifneq ($(filter %clean,$(MAKECMDGOALS)),)
INCLUDE_CLEAN_RDEP_SUBJECT	:= $(MAKECMDGOALS)
endif
endif

# Remove potential for circular dependencies.
INCLUDE_CLEAN_RDEPS		:= $(filter-out include-clean,\
						$(INCLUDE_CLEAN_RDEP_SUBJECT))

include-clean:: $(INCLUDE_CLEAN_RDEPS) $(abs_top_builddir)/include
	-$(MAKE) -C include -f "$(abs_top_srcdir)/include/Makefile" clean

# include-install is separate to avoid creating a circular dependency below in
# the install target.
.SECONDEXPANSION:
$(INSTALL_TARGETS) include-install lib-install: %-install: $(abs_top_builddir)/$$*
	$(MAKE) -C $* -f "$(abs_top_srcdir)/$*/Makefile" install 

# Just in case configure hasn't been run yet, let's not overambitiously remove
# the $(INSTALL_DIR).
clean:: $(CLEAN_TARGETS)
	$(RM) -f Version
	$(if $(DESTDIR)$(prefix),-$(RM) -Rf "$(INSTALL_DIR)")

SRCDIR_INSTALL_SCRIPTS	:= runltp runltplite.sh ver_linux
SRCDIR_INSTALL_READONLY	:= Version
SRCDIR_INSTALL_TARGETS	:= $(SRCDIR_INSTALL_SCRIPTS) $(SRCDIR_INSTALL_READONLY)

# Save space. We only need the first line in ChangeLog for runltp*.
Version: $(top_srcdir)/ChangeLog
	head -n 1 "$^" > "$@"

$(INSTALL_DIR)/Version: Version
	install -m 00644 "$(top_builddir)/$(@F)" "$@"

$(addprefix $(DESTDIR)/$(bindir)/,$(BINDIR_INSTALL_SCRIPTS)) \
$(addprefix $(INSTALL_DIR)/,$(SRCDIR_INSTALL_SCRIPTS)): %:
	install -m 00755 "$(top_srcdir)/$(@F)" "$@"

INSTALL_TARGETS		+= $(addprefix $(INSTALL_DIR)/,\
				$(SRCDIR_INSTALL_TARGETS))

$(INSTALL_TARGETS): $(INSTALL_DIR)

## Install
install: $(INSTALL_TARGETS)
ifeq ($(SKIP_IDCHECK),0)
	@CREATE_ENTRIES=$(CREATE_ENTRIES) \
	 DESTDIR="$(DESTDIR)" \
	 "$(top_srcdir)/IDcheck.sh"
else
	@echo "*******************************************************"
	@echo "** Will not run IDcheck.sh (SKIP_IDCHECK set to 1).  **"
	@echo "*******************************************************"
endif

## Misc targets.

## Help
.PHONY: help
help:
	@cat "$(top_srcdir)/README.mk-user"; exit 1

## Menuconfig
menuconfig:
	@$(SHELL) "$(top_srcdir)/ltpmenu"

## Package
package: 
	@$(RPMBUILD) -ba ltp-devel.spec

## End misc targets.
