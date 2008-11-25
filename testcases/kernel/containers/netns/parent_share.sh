#!/bin/sh

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

# This script is executed in the parent NS. 
# It binds and does sharable mount of sysfs .
#
#For child to refer parent sys
# set -x

# The test case ID, the test case count and the total number of test case

TCID=${TCID:-parent_share.sh}
TST_TOTAL=1
TST_COUNT=1
export TCID
export TST_COUNT
export TST_TOTAL
ret=0 
. initialize.sh

    mkdir -p /tmp/par_sysfs /tmp/mnt || ret=1
    mount --bind /sys /tmp/par_sysfs || ret=1
    
    #share parent namespace
    mount --bind /tmp/mnt /tmp/mnt || ret=1
    #mount --make-shared /mnt
    $smount /tmp/mnt shared > /dev/null || ret=1
    if [ $ret -ne 0 ] ; then
        tst_resm TFAIL "Error while doing shared mount"
        exit -1
    fi
