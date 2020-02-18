# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines Corp., 2001

NEEDSPECIAL	:= $(shell echo MAX_SWAPFILES | $(CC) -E -xc -include linux/swap.h 2>/dev/null - | tail -n 1 | grep 32; echo $?)
ifneq ($(strip $(NEEDSPECIAL)),)
export CFLAGS	+= -DOLDER_DISTRO_RELEASE
endif

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

FILTER_OUT_MAKE_TARGETS         := libswapon

include $(top_srcdir)/include/mk/generic_leaf_target.mk

$(MAKE_TARGETS): %: libswapon.o
