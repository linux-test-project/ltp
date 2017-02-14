#
#    realtime/func/pi-tests test suite Makefile.
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

top_srcdir		?= ../../../..

INSTALL_TARGETS		:= run_auto.sh parse-testpi1.py parse-testpi2.py
include $(top_srcdir)/include/mk/env_pre.mk
include $(abs_srcdir)/../../config.mk

MAKE_TARGETS		:= testpi-0 testpi-1 testpi-2 testpi-4 testpi-5 testpi-6 testpi-7 sbrk_mutex

include $(top_srcdir)/include/mk/generic_leaf_target.mk
