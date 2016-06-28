# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (C) 2009, Cisco Systems Inc.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it would be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

nfs04_create_file: CPPFLAGS += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
nfs05_make_tree: LDLIBS += -lpthread

INSTALL_TARGETS		:= nfs_lib.sh \
			   nfs01 \
			   nfs02 \
			   nfs03 \
			   nfs04 \
			   nfs05 \
			   nfs06

include $(top_srcdir)/include/mk/generic_leaf_target.mk
