# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2001

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

mprotect04: CFLAGS += -falign-functions=64

include $(top_srcdir)/include/mk/generic_leaf_target.mk
