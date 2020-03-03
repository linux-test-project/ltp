# SPDX-License-Identifier: GPL-2.0-or-later
#    testcases/kernel/syscalls Makefile.
#
#    Copyright (C) 2009, Cisco Systems Inc.
#
# Ngie Cooper, July 2009
#

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

CPPFLAGS		+= -D_XOPEN_SOURCE=500 -D_XOPEN_SOURCE_EXTENDED

MAKE_TARGETS		:= nftw01 nftw6401

nftw01: CPPFLAGS += -D_LARGEFILE_SOURCE

nftw01: nftw.o tools.o test_func.o test.o lib.o 
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) $(OUTPUT_OPTION)

nftw64.o tools64.o test_func64.o test64.o lib64.o: %.o: %.c

%64.o: CPPFLAGS += -D_LARGEFILE64_SOURCE

nftw6401: nftw64.o tools64.o test_func64.o test64.o lib64.o
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) $(OUTPUT_OPTION)

include $(top_srcdir)/include/mk/generic_leaf_target.mk
