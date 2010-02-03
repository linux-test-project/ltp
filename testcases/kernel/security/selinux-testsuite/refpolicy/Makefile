#
#    kernel/security/selinux-testsuite/refpolicy Makefile
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
# Garrett Cooper, January 2010
#

top_srcdir		?= ../../../../..

include	$(top_srcdir)/include/mk/env_pre.mk

ARGS			= -m
# Don't want to pass the -d option unless DESTDIR is a non-zero length string.
ifneq ($(strip $(DESTDIR)),)
ARGS			+= -d $(DESTDIR)
endif

DISTRO_VER		:= $(shell $(top_srcdir)/scripts/detect_distro.sh $(ARGS))

# Avoid empty strings.
ifeq ($(strip $(DISTRO_VER)),)
DISTRO_VER		:= generic
endif

CHECKPOLICY		?= $(DESTDIR)/usr/bin/checkpolicy
CHECKPOLICY_VERS	?= $(shell $(CHECKPOLICY) -V |cut -f 1 -d ' ')
POLICY_DEVEL_DIR	?= $(DESTDIR)/usr/share/selinux/devel

INSTALL_DIR		:= testcases/kernel/security/selinux-testsuite/refpolicy

TEST_POLICY_DIR		:= $(abs_srcdir)/policy_files

# Do we have a special set of policies in the SCM to install?
ifneq ($(wildcard $(TEST_POLICY_DIR)/$(DISTRO_VER)/),)
TEST_POLICY_DIR		:= $(TEST_POLICY_DIR)/$(DISTRO_VER)
else
TEST_POLICY_DIR		:= $(TEST_POLICY_DIR)/generic
endif

MAKE_TARGETS		:= test_policy.te test_policy.pp

# This is being done to preserve precedence; test_global.te must come first.
POLICY_FILES		:= test_global.te \
			   $(filter-out test_global.te,$(notdir $(wildcard $(TEST_POLICY_DIR)/*.te)))

ifneq ($(CHECKPOLICY_VERS),24)
POLICY_FILES		:= $(filter-out test_bounds.te,$(POLICY_FILES))
endif

test_policy.te: $(addprefix $(TEST_POLICY_DIR)/,$(POLICY_FILES))
	(cd "$(TEST_POLICY_DIR)" && cat $(POLICY_FILES)) > $@

test_policy.pp: test_policy.te
	set -e; if [ -d "$(POLICY_DEVEL_DIR)" ]; then \
		$(MAKE) -f $(POLICY_DEVEL_DIR)/Makefile test_policy.pp; \
	else \
		echo "ERROR: You must have selinux-policy?-devel? installed."; \
		false; \
	fi

include $(top_srcdir)/include/mk/generic_leaf_target.mk
