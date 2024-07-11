# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2009, Cisco Systems Inc.
# Copyright (c) Linux Test Project, 2009-2024
# realtime/func/sched_football test suite Makefile.
# Ngie Cooper, September 2009

top_srcdir		?= ../../../..

INSTALL_TARGETS		:= run_auto.sh
include $(top_srcdir)/include/mk/env_pre.mk
include $(abs_srcdir)/../../config.mk

# TODO: integrate properly with LTP library
LDLIBS			+= -lltp
include $(top_srcdir)/include/mk/generic_leaf_target.mk
