# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
# Copyright (c) 2018 Xiao Yang <yangx.jy@cn.fujitsu.com>
#

ifneq ($(KERNELRELEASE),)

obj-m := dummy_del_mod.o dummy_del_mod_dep.o

else

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

REQ_VERSION_MAJOR	:= 2
REQ_VERSION_PATCH	:= 6

MAKE_TARGETS		:= delete_module01 delete_module02 delete_module03 \
			   dummy_del_mod.ko dummy_del_mod_dep.ko

include $(top_srcdir)/include/mk/module.mk
include $(top_srcdir)/include/mk/generic_leaf_target.mk

endif
