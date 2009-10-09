#
#    kernel/security/selinux-testsuite include Makefile
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

CURDIR_DIRNAME		:= $(lastword $(subst /, ,$(abs_srcdir)))

LDLIBS			+= -lselinux -lpthread

# This is a soft error because it won't work if you're not root and doing a
# cross-compilation based install.
define INSTALL_POST
	-chcon -t test_file_t $@
endef

INSTALL_TARGETS		:= selinux_$(CURDIR_DIRNAME)*.sh

# vim: syntax=make
