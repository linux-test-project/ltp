# SPDX-License-Identifier: GPL-2.0-or-later
#  Copyright (c) 2016 RT-RK Institute for Computer Based Systems
#  Author: Dejan Jovicevic <dejan.jovicevic@rt-rk.com>

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

listxattr04: LDLIBS	+= $(ACL_LIBS)

include $(top_srcdir)/include/mk/generic_leaf_target.mk
