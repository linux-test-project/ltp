# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2009, Cisco Systems Inc.
# Ngie Cooper, July 2009

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/env_pre.mk

INSTALL_TARGETS		:= create_file tst_net_stress.sh ns-icmpv4_sender

FILTER_OUT_MAKE_TARGETS	:= ns-common

include $(top_srcdir)/include/mk/generic_leaf_target.mk

$(MAKE_TARGETS): %: ns-common.o
