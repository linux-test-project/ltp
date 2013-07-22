# Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it would be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write the Free Software Foundation,
# Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

ifneq ($(KERNELRELEASE),)

ifdef CONFIG_FW_LOADER
obj-m		:= ltp_fw_load.o
endif

else

top_srcdir	?= ../../../..
include $(top_srcdir)/include/mk/env_pre.mk

MAKE_TARGETS	:= ltp_fw_load.ko

# Ignoring the exit status of commands is done to be forward compatible with
# kernel internal API changes. The user-space test will return TCONF, if it
# doesn't find the module (i.e. it wasn't built either due to kernel-devel
# missing or module build failure).
ltp_fw_load.ko: ltp_fw_load.c
	-$(MAKE) -C $(LINUX_DIR) M=$(abs_srcdir)
	-mv ltp_fw_load.ko ltp_fw_load.ko~
	-$(MAKE) -C $(LINUX_DIR) M=$(abs_srcdir) clean
	-mv ltp_fw_load.ko~ ltp_fw_load.ko

include $(top_srcdir)/include/mk/generic_leaf_target.mk

endif
