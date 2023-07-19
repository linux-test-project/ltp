# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2009, Cisco Systems Inc.
# Ngie Cooper, July 2009

top_srcdir		?= ..

include $(top_srcdir)/include/mk/env_pre.mk

CFLAGS			+= -I. -DLTPLIB

ifneq ($(ANDROID),1)
FILTER_OUT_DIRS		+= android_libpthread android_librt
else
FILTER_OUT_LIBSRCS	+= tlibio.c tst_safe_sysv_ipc.c
endif

INTERNAL_LIB		:= libltp.a

pc_file			:= $(DESTDIR)/$(datarootdir)/pkgconfig/ltp.pc

INSTALL_TARGETS		:= $(pc_file)

tst_test.o: ltp-version.h

ltp-version.h: gen_version

MAKE_TARGETS += gen_version

.PHONY: gen_version
gen_version:
	@echo GEN ltp-version.h
	@$(top_srcdir)/lib/gen_version.sh

CLEAN_TARGETS += ltp-version.h cached-version

$(pc_file):
	test -d "$(@D)" || mkdir -p "$(@D)"
	install -m $(INSTALL_MODE) "$(builddir)/$(@F)" "$@"

include $(top_srcdir)/include/mk/lib.mk
include $(top_srcdir)/include/mk/generic_trunk_target.mk
