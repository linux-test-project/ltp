# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2009, Cisco Systems Inc.
# Ngie Cooper, July 2009

top_srcdir		?= ../../../../..

include $(top_srcdir)/include/mk/testcases.mk
include $(abs_srcdir)/../Makefile.inc
include $(top_srcdir)/include/mk/generic_leaf_target.mk

CFLAGS_no_stack_prot := $(filter-out -fstack-clash-protection, $(CFLAGS))

hugemmap06: CFLAGS+=-pthread
hugemmap34: CFLAGS=$(CFLAGS_no_stack_prot)
