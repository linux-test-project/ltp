# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2015 Fujitsu Ltd.
# Author:Guangwen Feng <fenggw-fnst@cn.fujitsu.com>

ifneq ($(KERNELRELEASE),)

obj-m := ltp_lsmod01.o

else

top_srcdir		?= ../../..

include $(top_srcdir)/include/mk/testcases.mk

REQ_VERSION_MAJOR       := 2
REQ_VERSION_PATCH       := 6
MAKE_TARGETS            := ltp_lsmod01.ko

include $(top_srcdir)/include/mk/env_pre.mk

INSTALL_TARGETS		:= lsmod01.sh

include $(top_srcdir)/include/mk/module.mk
include $(top_srcdir)/include/mk/generic_leaf_target.mk

endif
