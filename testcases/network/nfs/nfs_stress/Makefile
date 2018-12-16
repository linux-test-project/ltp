# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2015-2018 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (C) 2009, Cisco Systems Inc.

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
