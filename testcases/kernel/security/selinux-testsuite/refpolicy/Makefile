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
# Garrett Cooper, August 2009
#

top_srcdir		?= ../../../../..

include	$(top_srcdir)/include/mk/env_pre.mk

REDHAT_RELEASE		?= $(shell rpm -q redhat-release)
REDHAT_VERS		?= $(shell echo $(REDHAT_RELEASE) | cut -f3 -d- | sed -e "s/[^0-9]*//g")
POLICYDEVEL		?= $(DESTDIR)/usr/share/selinux/devel
SEMODULE		?= $(DESTDIR)/usr/sbin/semodule
CHECKPOLICY		?= $(DESTDIR)/usr/bin/checkpolicy
CHECKPOLICY_VERS	?= $(shell $(CHECKPOLICY) -V |cut -f 1 -d ' ')

CLEAN_TARGETS		:= test_policy.te

INSTALL_TARGETS		:= $(REDHAT_VERS)/*.te
INSTALL_TARGETS_FULL	:= redhat/$(INSTALL_TARGETS)

ifeq ($(CHECKPOLICY_VERS),24)
INSTALL_TARGETS		:= $(filter-out %/test_bounds.te,$(INSTALL_TARGETS))
endif

TE_SRCDIR		:= $(abs_srcdir)

ifeq (redhat-release-, $(findstring redhat-release-, $(REDHAT_RELEASE)))
ifneq ($(wildcard $(abs_srcdir)/redhat/$(REDHAT_VER)),)
TE_SRCDIR		:= $(abs_srcdir)/redhat/$(REDHAT_VER)
endif
endif

.PHONY: all clean cleanup install load

all: load

clean: cleanup

cleanup:
	-$(SEMODULE) -r test_policy
	$(RM) -f $(POLICYDEVEL)/test_policy.* test_policy.te 

install: all

# load remains for backwards compatibility...
load: test_policy.te
ifeq (redhat-release-, $(findstring redhat-release-, $(REDHAT_RELEASE)))
	$(MAKE) -C redhat/$(REDHAT_VERS)
else
	@if [ -d "$(POLICYDEVEL)" ]; then \
	    cp -p test_policy.* $(INSTALL_TARGETS_FULL) $(POLICYDEVEL); \
	    $(MAKE) -C $(POLICYDEVEL) clean test_policy.pp; \
	    $(SEMODULE) -i $(POLICYDEVEL)/test_policy.pp; \
	else \
            echo "ERROR: You must have selinux-policy-devel installed."; \
	fi
endif

test_policy.te:
	(cd "$(TE_SRCDIR)" && cat $(filter-out $@,$(addprefix $(srcdir)/,$(INSTALL_TARGETS))) > "$@";
