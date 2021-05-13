# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2009, Cisco Systems Inc.
# Ngie Cooper, July 2009

top_srcdir		?= ../../../../../..

include $(top_srcdir)/include/mk/testcases.mk

ima_boot_aggregate: LDLIBS += $(CRYPTO_LIBS) -ldl

include $(top_srcdir)/include/mk/generic_leaf_target.mk
