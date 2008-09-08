#!/bin/sh
################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2001                 ##
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
################################################################################
## File:        runalltests.sh                                                ##
##                                                                            ##
## Description:  This script just calls runltp now, and is being phased out.  ##
##		If you rely on this script for automation reasons, please     ##
##                                                                            ## 
## History	Subrata Modak <subrata@linuc.vnet.ibm.com> changed the code   ##
##		to include testing other testcases which are not run by       ##
##		default, 08/09/2008                                           ##
##                                                                            ##
################################################################################

echo "*******************************************************************"
echo "*******************************************************************"
echo "**                                                               **"
echo -e "** This script is being re-written to cover all aspects of    **"
echo -e "** testing LTP, which includes running all those tests which  **"
echo -e "** are not run by default with ./runltp script. Special setup **"
echo -e "** in system environment will be done to run all those tests  **"
echo -e "** like the File System tests, SELinuxtest, etc               **"
echo "**                                                               **"
echo "*******************************************************************"
echo "*******************************************************************"
