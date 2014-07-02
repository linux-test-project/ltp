#
#    Copyright (c) 2014 Fujitsu Ltd.
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

top_srcdir                      ?= ../../../../..

include $(top_srcdir)/include/mk/env_pre.mk

INSTALL_TARGETS		:= cpuhotplug_do_disk_write_loop cpuhotplug_do_spin_loop \
					   cpuhotplug_do_kcompile_loop cpuhotplug_report_proc_interrupts

include $(top_srcdir)/include/mk/generic_leaf_target.mk
