################################################################################
##   kernel/io/stress_floppy/datafiles/dumpdir Makefile.                      ##
##                                                                            ##
##   Copyright (c) International Business Machines  Corp., 2001               ##
##   Copyright (c) 2015 Fujitsu Ltd.                                          ##
##                                                                            ##
##   This program is free software; you can redistribute it and/or modify     ##
##   it under the terms of the GNU General Public License as published by     ##
##   the Free Software Foundation; either version 2 of the License, or        ##
##   (at your option) any later version.                                      ##
##                                                                            ##
##   This program is distributed in the hope that it will be useful,          ##
##   but WITHOUT ANY WARRANTY; without even the implied warranty of           ##
##   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            ##
##   GNU General Public License for more details.                             ##
##                                                                            ##
##   You should have received a copy of the GNU General Public License along  ##
##   with this program; if not, write to the Free Software Foundation, Inc.,  ##
##   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.              ##
##                                                                            ##
################################################################################

top_srcdir		?= ../../../../../..

include $(top_srcdir)/include/mk/env_pre.mk

CLEAN_TARGETS		:= 10*K_file

MAKE_TARGETS		:= 10K_file 100K_file 1000K_file

INSTALL_DIR		:= testcases/data/stress_floppy/dumpdir

INSTALL_TARGETS		:= *K_file

10K_file:
100K_file:
1000K_file:
	@$(SHELL) "$(abs_srcdir)/generate.sh"

include $(top_srcdir)/include/mk/generic_leaf_target.mk
