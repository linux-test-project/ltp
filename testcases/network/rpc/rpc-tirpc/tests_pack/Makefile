# Copyright (C) 2014, Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>

top_srcdir		?= ../../../../..

include	$(top_srcdir)/include/mk/env_pre.mk

LIBDIR			:= lib
FILTER_OUT_DIRS		:= $(LIBDIR)
LIB			:= $(LIBDIR)/librpc-tirpc.a

# keep only rpc_*
ifeq ($(TIRPC_LIBS),)
FILTER_OUT_MAKE_TARGETS	+= tirpc_cleaner
FILTER_OUT_DIRS	+= tirpc_svc_1 tirpc_svc_2 tirpc_svc_3 tirpc_svc_4 tirpc_svc_5 tirpc_svc_6 tirpc_svc_7 tirpc_svc_8 tirpc_svc_9 tirpc_svc_11
endif


$(LIBDIR):
	mkdir -p "$@"

$(LIB): $(LIBDIR)
	$(MAKE) -C $^ -f "$(abs_srcdir)/$^/Makefile" all

trunk-clean:: | lib-clean

lib-clean:: $(LIBDIR)
	$(MAKE) -C $^ -f "$(abs_srcdir)/$^/Makefile" clean

include $(abs_srcdir)/Makefile.inc
include $(top_srcdir)/include/mk/generic_trunk_target.mk
