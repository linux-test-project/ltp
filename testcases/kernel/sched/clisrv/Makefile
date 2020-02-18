# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2009, Cisco Systems Inc.
# Ngie Cooper, July 2009

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/env_pre.mk

CPPFLAGS		+= -D_LINUX
LDLIBS			+= -lpthread

INSTALL_TARGETS		:= data run_sched_cliserv.sh

MAKE_TARGETS		:= pthcli pthserv

$(MAKE_TARGETS): %: readline.o writen.o

include $(top_srcdir)/include/mk/generic_leaf_target.mk
