# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.

ifneq ($(KERNELRELEASE),)

obj-m := ltp_block_dev.o

else

top_srcdir	?= ../../../../..
include $(top_srcdir)/include/mk/env_pre.mk

REQ_VERSION_MAJOR	:= 2
REQ_VERSION_PATCH	:= 6
MAKE_TARGETS		:= ltp_block_dev.ko

include $(top_srcdir)/include/mk/module.mk
include $(top_srcdir)/include/mk/generic_leaf_target.mk

endif
