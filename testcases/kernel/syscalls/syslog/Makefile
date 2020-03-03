# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2001

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

# Not all of the syslog* files are bourne shell scripts.
INSTALL_TARGETS		:= syslog0* syslog10 syslog-lib.sh

include $(top_srcdir)/include/mk/generic_leaf_target.mk
