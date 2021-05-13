# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2014 Fujitsu Ltd.

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

CPPFLAGS		+= -DETH_P_CAN=0x000C -DPF_CAN=29 -DAF_CAN=PF_CAN

include $(top_srcdir)/include/mk/generic_leaf_target.mk
