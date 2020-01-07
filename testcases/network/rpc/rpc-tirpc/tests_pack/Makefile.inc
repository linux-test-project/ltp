# Copyright (C) 2014, Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>

LIBRELDIR	:= testcases/network/rpc/rpc-tirpc/tests_pack/lib
LIBDIR		:= $(abs_top_builddir)/$(LIBRELDIR)
LIBSRCDIR	:= $(abs_top_srcdir)/$(LIBRELDIR)
LIBRPC-TIRPC	:= $(LIBDIR)/librpc-tirpc.a

$(LIBDIR):
	mkdir -p "$@"

$(LIBRPC-TIRPC): $(LIBDIR)
	$(MAKE) -C $^ -f "$(LIBSRCDIR)/Makefile" all

MAKE_DEPS	+= $(LIBRPC-TIRPC)

LDFLAGS		+= -L$(LIBDIR)
LDLIBS		+= $(TIRPC_LIBS) -lrpc-tirpc
CFLAGS		+= $(TIRPC_CFLAGS) -I$(LIBSRCDIR) -I$(LIBSRCDIR)/../include -pthread
