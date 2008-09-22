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

export LTP_VERSION=`./runltp -e`
export TEST_START_TIME=`date +"%Y_%b_%d-%Hh_%Mm_%Ss"`
export HARDWARE_TYPE=$(uname -i)
export HOSTNAME=$(uname -n)
export KERNEL_VERSION=$(uname -r)
export HTML_OUTPUT_FILE_NAME=$LTP_VERSION-$HOSTNAME-$KERNEL_VERSION-$HARDWARE_TYPE-$TEST_START_TIME.html


## The First one i plan to run is the default LTP run ##
## START => Test Series 1                             ##
echo -e "Running Default LTP..."
./runltp -g $HTML_OUTPUT_FILE_NAME
echo -e "Completed running Default LTP\n\n"
## END => Test Series 1                               ##

## The next one i plan to run is ballista             ##
## START => Test Series 2                             ##
echo -e "Running Ballista..."
export TEST_START_TIME=`date +"%Y_%b_%d-%Hh_%Mm_%Ss"`
./runltp -f ballista -o $LTP_VERSION-BALLISTA_RUN_ON-$HOSTNAME-$KERNEL_VERSION-$HARDWARE_TYPE-$TEST_START_TIME.out
echo -e "Completed running Ballista\n\n"
## END => Test Series 2                               ##

## The next one i plan to run is open_posix_testsuite ##
## START => Test Series 3                             ##
echo -e "Running Open Posix Tests..."
(cd testcases/open_posix_testsuite; make)
echo -e "Completed running Open Posix Tests\n\n"
## END => Test Series 3                               ##

