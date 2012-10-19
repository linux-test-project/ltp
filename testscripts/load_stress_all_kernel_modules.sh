#!/bin/sh
################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2009                 ##
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
################################################################################
#                                                                             ##
# File :        load_stress_all_kernel_modules.sh                             ##
#                                                                             ##
# Description:  Try to load all the modules present in the system, installed  ##
#               both during Distro installation, or, custom kernel build.     ##
#                                                                             ##
# Author:       Subrata Modak <subrata@linux.vnet.ibm.com>                    ##
################################################################################

for module in `modprobe -l | tr '\n' ' '`
  do
    insert_module=`basename $module .ko`
    modprobe -v $insert_module
done

