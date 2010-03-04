#
#    kernel/hotplug/cpu_hotplug test suite Makefile.
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
# Maxin B. John, March 2010

top_srcdir                      ?= ../../../..

include $(top_srcdir)/include/mk/env_pre.mk

# Avoid namespace collisions because the installation filenames are pretty
# generic.
INSTALL_DIR			:= testcases/bin/cpu_hotplug

INSTALL_TARGETS                 := runtests.sh functional/* include/* tools/*

include $(top_srcdir)/include/mk/generic_leaf_target.mk

