# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2009, Cisco Systems Inc.
# Ngie Cooper, July 2009

top_srcdir		?= ../../..

include $(top_srcdir)/include/mk/env_pre.mk

INSTALL_TARGETS		:= *.sh

include $(top_srcdir)/include/mk/generic_trunk_target.mk

# librpc-tirpc.a in rpc-tirpc/ must be built before basic_tests
ifneq ($(filter rpc-tirpc,$(SUBDIRS)),)
basic_tests-all basic_tests-install: rpc-tirpc-all
endif
