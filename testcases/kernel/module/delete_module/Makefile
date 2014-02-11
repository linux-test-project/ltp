#
# Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of version 2 of the GNU General Public License as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it would be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
###########################################################################
# name of file		 : Makefile		 		 	  #
# description		 : make file for the delete_module(2) testcases	  #
###########################################################################

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
