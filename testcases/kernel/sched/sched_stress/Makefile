# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2009, Cisco Systems Inc.
# Ngie Cooper, July 2009

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

INSTALL_TARGETS		:= sched_stress.sh

LDLIBS			+= -lpthread

MAKE_TARGETS		:= $(filter-out sched,$(patsubst $(abs_srcdir)/%.c,%,$(wildcard $(abs_srcdir)/*.c)))

RM			+= -r

$(filter-out sched_datafile,$(MAKE_TARGETS)): %: sched.o

MAKE_TARGETS		+= sched_datafile

sched_datafile:
	"$(top_srcdir)/tools/make-file.sh" "$@" 1200010

include $(top_srcdir)/include/mk/generic_leaf_target.mk
