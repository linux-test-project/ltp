# SPDX-License-Identifier: GPL-2.0-or-later
# Copied from ../open/Makefile by Masatake YAMATO
#
###
#
#  Copyright (c) International Business Machines  Corp., 2001

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk
include $(abs_srcdir)/../utils/newer_64.mk

%_64: CPPFLAGS += -D_FILE_OFFSET_BITS=64 -D$(DEF_64)=1

include $(top_srcdir)/include/mk/generic_leaf_target.mk
