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
# with this program; if not, write the Free Software Foundation, Inc., 59
# Temple Place - Suite 330, Boston MA 02111-1307, USA.
#
###########################################################################
# name of file		 : Makefile		 		 	  #
# description		 : make file for the delete_module(2) testcases	  #
###########################################################################

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

CFLAGS			+= -D_GNU_SOURCE

LOCALVERSION		?= $(shell uname -r)

MODULEDIR		:= $(DESTDIR)/lib/modules/$(LOCALVERSION)/build

# KBuild make variables...
MODCFLAGS		+= $(MODULEDIR)

export EXTRA_CFLAGS	:= $(CFLAGS)
export EXTRA_CPPFLAGS	:= $(CPPFLAGS)
export EXTRA_LDFLAGS	:= $(LDFLAGS)

obj-m			:= dummy_del_mod.o dummy_del_mod_dep.o
MODULES			:= dummy_del_mod.ko dummy_del_mod_dep.ko

# New make infra variables...
CLEAN_TARGETS		:= .dummy* *.mod.c .tmp_version *.ko
INSTALL_TARGETS		:= $(MODULES)
MAKE_TARGETS		:= delete_module01 delete_module02 delete_module03

include $(top_srcdir)/include/mk/generic_leaf_target.mk

$(MAKE_TARGETS): | make-module

# KBuild will gripe because we redefine CFLAGS if KBUILD_NOPEDANTIC isn't set.
.PHONY: make-module
make-module:
	make -C $(MODULEDIR) KBUILD_NOPEDANTIC=1 SUBDIRS=$(abs_srcdir) modules
