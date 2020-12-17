# SPDX-License-Identifier: GPL-2.0-or-later

ifneq ($(KERNELRELEASE),)

obj-m := init_module.o

else

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

REQ_VERSION_MAJOR	:= 2
REQ_VERSION_PATCH	:= 6

MAKE_TARGETS		:= init_module01 init_module02 init_module.ko

include $(top_srcdir)/include/mk/module.mk
include $(top_srcdir)/include/mk/generic_leaf_target.mk

endif
