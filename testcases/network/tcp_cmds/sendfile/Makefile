# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2009, Cisco Systems Inc.
# Ngie Cooper, September 2009

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk
include $(abs_srcdir)/../Makefile.inc

CPPFLAGS		+= -I$(abs_srcdir)/../include

INSTALL_TARGETS		:= sendfile01.sh

MAKE_TARGETS		+= testsf_c testsf_s testsf_c6 testsf_s6

testsf_c6.o testsf_s6.o: CPPFLAGS += -DINET6

testsf_c6.o testsf_s6.o: %6.o: %.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

include $(top_srcdir)/include/mk/generic_leaf_target.mk
