#
#    Copyright (c) 2015 Fujitsu Ltd.
#    Author:Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#

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
