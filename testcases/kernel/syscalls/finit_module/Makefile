# SPDX-License-Identifier: GPL-2.0-or-later

ifneq ($(KERNELRELEASE),)

obj-m := finit_module.o

else

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

REQ_VERSION_MAJOR	:= 3
REQ_VERSION_PATCH	:= 8

MAKE_TARGETS		:= finit_module01 finit_module02 finit_module.ko

include $(top_srcdir)/include/mk/module.mk
include $(top_srcdir)/include/mk/generic_leaf_target.mk

endif
