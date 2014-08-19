################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2008                 ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    ##
##                                                                            ##
## Author:      Veerendra <veeren@linux.vnet.ibm.com>                         ##
################################################################################

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk
include $(abs_srcdir)/../Makefile.inc

INSTALL_TARGETS		:= *.sh netns_container_ftp.pl

LDLIBS			+= -lclone

MAKE_TARGETS		:= netns_create_container netns_crtchild \
			   netns_crtchild_delchild netns_par_chld_ftp \
			   netns_par_chld_ipv6 netns_sysfsview \
			   netns_two_children_ns

$(MAKE_TARGETS): %: common.o %.o

include $(top_srcdir)/include/mk/generic_leaf_target.mk
