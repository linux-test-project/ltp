#
#    network test suite Makefile.
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
# Ngie Cooper, July 2009
#

top_srcdir		?= ../..

include $(top_srcdir)/include/mk/env_pre.mk

DIR			:= datafiles

CLEAN_TARGETS		+= $(DIR)

INSTALL_DIR		:= testcases/bin

INSTALL_TARGETS		:= $(addprefix $(DIR)/ascii.,sm med lg jmb)
INSTALL_TARGETS		+= $(addprefix $(DIR)/bin.,sm med lg jmb)

RM			+= -r

ifeq ($(ANDROID),1)
FILTER_OUT_DIRS		+= lib6 rpc sockets
endif

$(INSTALL_TARGETS): | generate

.PHONY: generate

include $(top_srcdir)/include/mk/generic_trunk_target.mk

$(DESTDIR)/$(INSTALL_DIR)/$(DIR) $(DIR): %:
	mkdir -p "$@"

generate: $(DIR)
	cd "$(DIR)" && \
	abs_top_srcdir="$(abs_top_srcdir)" "$(abs_srcdir)/generate.sh"
