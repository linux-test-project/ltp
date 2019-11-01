# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2009, Cisco Systems Inc.
# Copyright (c) Linux Test Project, 2006-2020
# Ngie Cooper, October 2009

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

INSTALL_TARGETS		+= route[4-6]-* *.sh

route-change-netlink: CFLAGS += $(LIBMNL_CFLAGS)
route-change-netlink: LDLIBS += $(LIBMNL_LIBS)

include $(top_srcdir)/include/mk/generic_leaf_target.mk
