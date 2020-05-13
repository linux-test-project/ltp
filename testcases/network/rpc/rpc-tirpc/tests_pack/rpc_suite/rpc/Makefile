# Copyright (C) 2014, Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>

top_srcdir		?= ../../../../../../..

include	$(top_srcdir)/include/mk/env_pre.mk

ifeq ($(TIRPC_LIBS),)
# glibc does not define registerrpc() in <rpc/rpc.h>
# registerrpc() is deprecated in libtirpc, it should be replaced by rpc_reg()
FILTER_OUT_DIRS	+= rpc_regunreg_registerrpc
endif

include	$(top_srcdir)/include/mk/env_pre.mk
include $(top_srcdir)/include/mk/generic_trunk_target.mk
