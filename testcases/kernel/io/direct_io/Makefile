# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines Corp., 2001

top_srcdir			?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

CFLAGS				+= -DSHARED_OFILE -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE

LDLIBS				+= -lpthread

FILTER_OUT_MAKE_TARGETS		:= diotest_routines

INSTALL_TARGETS			:= test_dma_thread_diotest7.sh

include $(top_srcdir)/include/mk/generic_leaf_target.mk

$(MAKE_TARGETS): %: diotest_routines.o
