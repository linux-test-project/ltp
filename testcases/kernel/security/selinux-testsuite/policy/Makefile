#
#    kernel/security/selinux-testsuite/policy Makefile
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
include $(DESTDIR)/etc/selinux/config

SELINUX_SRC		?= $(DESTDIR)/etc/selinux/$(SELINUXTYPE)/src/policy

# cleanup and load remain for backwards compatibility...
.PHONY: all clean cleanup install load

all: load

clean: | cleanup

cleanup:
	$(RM) -f $(SELINUX_SRC)/domains/misc/test_*
	@if [ -d "$(SELINUX_SRC)" ]; then \
	    $(MAKE) -C $(SELINUX_SRC) -W users load; \
	fi

install: all

load:
	@if [ -d "$(SELINUX_SRC)" ]; then \
	    cp -p $(abs_srcdir)/test_* "$(SELINUX_SRC)/domains/misc/"; \
	    $(MAKE) -C "$(SELINUX_SRC)" -W users $@; \
	else \
	    echo "ERROR: You must have selinux-policy-targeted-sources " \
	         "installed."; \
	fi
