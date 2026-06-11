# Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) Linux Test Project, 2026
# SPDX-License-Identifier: GPL-2.0-or-later

ifneq ($(KERNELRELEASE),)

ifdef CONFIG_FW_LOADER
obj-m		:= ltp_fw_load.o
endif

else

top_srcdir	?= ../../../..
include $(top_srcdir)/include/mk/env_pre.mk

REQ_VERSION_MAJOR   := 3
REQ_VERSION_PATCH   := 7
MAKE_TARGETS	:= ltp_fw_load.ko

include $(top_srcdir)/include/mk/module.mk
include $(top_srcdir)/include/mk/generic_leaf_target.mk

endif
