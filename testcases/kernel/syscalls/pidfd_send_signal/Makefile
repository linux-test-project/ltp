// SPDX-License-Identifier: GPL-2.0-or-later
#
#   Copyright (c) 2019 SUSE LLC
#
#   Author: Christian Amann <camann@suse.com>
#

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

include $(top_srcdir)/include/mk/generic_leaf_target.mk

pidfd_send_signal01: CFLAGS += -pthread
