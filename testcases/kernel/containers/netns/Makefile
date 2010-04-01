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
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
## Author:      Veerendra <veeren@linux.vnet.ibm.com>                         ##
################################################################################

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk
include $(abs_srcdir)/../Makefile.inc

INSTALL_TARGETS		:= *.sh container_ftp.pl

MAKE_TARGETS		:= create_container crtchild crtchild_delchild \
			   par_chld_ftp par_chld_ipv6 sysfsview two_children_ns

$(MAKE_TARGETS): %: common.o %.o

MAKE_TARGETS		+= check_netns_enabled

include $(top_srcdir)/include/mk/generic_leaf_target.mk
