# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2009, Cisco Systems Inc.
# Author: Ngie Cooper, September 2009

top_srcdir		?= ../../../../..

include $(top_srcdir)/include/mk/testcases.mk

CPPFLAGS		+= -I$(abs_srcdir)/../../libcontrollers

INSTALL_TARGETS		:= *.sh

LDLIBS			+= -lm

include $(top_srcdir)/include/mk/generic_leaf_target.mk
