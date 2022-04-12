# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2021 SUSE LLC <rpalethorpe@suse.com>

top_srcdir		?= ../..

include $(top_srcdir)/include/mk/env_pre.mk
include $(top_srcdir)/include/mk/functions.mk

SPARSE_SRC	?= sparse-src

$(SPARSE_SRC)/Makefile:
ifeq ($(SPARSE_SRC),sparse-src)
	git submodule update --init
else
	$(error "Can't find $(SPARSE_SRC)/Makefile")
endif

$(SPARSE_SRC)/libsparse.a: $(SPARSE_SRC)/Makefile
	$(MAKE) -C $(SPARSE_SRC) libsparse.a

HOST_MAKE_TARGETS	:= sparse-ltp
MAKE_DEPS		+= $(SPARSE_SRC)/libsparse.a
HOST_CFLAGS		+= -I$(SPARSE_SRC) -Werror -Wno-null-pointer-subtraction
HOST_LDLIBS		+= $(SPARSE_SRC)/libsparse.a


include $(top_srcdir)/include/mk/generic_leaf_target.mk
