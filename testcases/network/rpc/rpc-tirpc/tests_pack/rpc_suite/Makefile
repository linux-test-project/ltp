# Copyright (C) 2009, Cisco Systems Inc.
# Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>

top_srcdir		?= ../../../../../..

include	$(top_srcdir)/include/mk/env_pre.mk

ifeq ($(TIRPC_LIBS),)
FILTER_OUT_DIRS	+= tirpc
endif

include	$(top_srcdir)/include/mk/env_pre.mk
include $(top_srcdir)/include/mk/generic_trunk_target.mk
