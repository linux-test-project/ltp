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

# This script propagates the child sysfs contents to be visible for parent
# Also it will check the parent sysfs contents are visible.
#Propagate child sys directory

# The test case ID, the test case count and the total number of test case
TCID=${TCID:-child_propagate.sh}
TST_TOTAL=1
TST_COUNT=1
#set -x
export TCID
export TST_COUNT
export TST_TOTAL

    ret=0
    PROPAGATE=`cat /tmp/FIFO4`
    debug "INFO: CHILD propagated.."
    mount -t sysfs none /sys || ret=1
    mkdir -p /tmp/mnt/sys || ret=1
    mount --bind /sys /tmp/mnt/sys > /dev/null || ret=1
    
    if [ $ret -ne 0 ]; then
        status=1
        tst_resm TFAIL "error while doing bind mount"
        exit $status
    fi
    #Capture childs sysfs contents
    ls /sys/class/net > /tmp/child_sysfs
    echo propagated > /tmp/FIFO5

    #Capture parent sysfs in child
    ls /tmp/par_sysfs/class/net > /tmp/parent_sysfs_in_child
    diff /tmp/parent_sysfs_in_child /tmp/parent_sysfs > /dev/null 2>&1
    if [ $? -eq 0 ]
    then
        tst_resm TINFO "Pass:Child is able to view parent sysfs"
        status=0
    else
        tst_resm TFAIL "Fail:Child view of sysfs is not same as parent sysfs"
        status=1
    fi

    #cleanup
    rm -f /tmp/parent_sysfs_in_child /tmp/parent_sysfs 
    umount /tmp/mnt/sys
    rm -rf /tmp/mnt  > /dev/null 2>&1 || true
