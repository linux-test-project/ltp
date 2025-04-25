# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2009, Cisco Systems Inc.
# Ngie Cooper, July 2009

top_srcdir		?= ..

include $(top_srcdir)/include/mk/env_pre.mk

INSTALL_DIR		:= $(includedir)

INSTALL_MODE		:= 00644

INSTALL_TARGETS		:= *.h

MAKE_TARGETS		:=

.PHONY: ac-clean ac-distclean ac-maintainer-clean distclean maintainer-clean
distclean:: clean ac-distclean
maintainer-clean:: distclean ac-maintainer-clean
ac-clean ac-distclean::
	$(RM) -f config.h lapi/syscalls.h stamp-h1
ac-maintainer-clean:: ac-clean
	$(RM) -f config.h.in

vpath %.h $(abs_srcdir)

include $(top_srcdir)/include/mk/generic_leaf_target.mk
