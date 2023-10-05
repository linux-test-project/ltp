# SPDX-License-Identifier: GPL-2.0-or-later
# Copied from ../getpid/Makefile by Masatake YAMATO
#
###
#
#  Copyright (c) International Business Machines  Corp., 2001

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

ifeq ($(ANDROID), 1)
FILTER_OUT_MAKE_TARGETS	+= gettid01 gettid02
endif

gettid02: LDLIBS += -lpthread

include $(top_srcdir)/include/mk/generic_leaf_target.mk
