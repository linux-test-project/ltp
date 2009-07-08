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

# This script deletes the child ns, and checks the device 
# returned to the parent ns.

# Reading contents of the sys fs to file in Parent
# set -x

# The test case ID, the test case count and the total number of test case

TCID=${TCID:-delchild.sh}
TST_TOTAL=1
TST_COUNT=1
export TCID
export TST_COUNT
export TST_TOTAL
    
    sshpid=`cat /tmp/FIFO3`
    debug "INFO: ssh pid is  $sshpid"
    newnet=`cat /tmp/FIFO4`
    debug "INFO: new dev is  $newnet"

    if [ $newnet = -1 ] ; then
        status=-1
    fi
    
    ls /sys/class/net > /tmp/sys_b4_child_killed
    sleep 2
    
    debug "INFO: Deleting the child NS created.. "
    debug "INFO: Killing processes $sshpid $pid"
    kill -9 $sshpid $pid > /dev/null 2>&1
    sleep 1
    
    ls /sys/class/net > /tmp/sys_aftr_child_killed
    diff -q /tmp/sys_b4_child_killed /tmp/sys_aftr_child_killed 
    
    if [ $? = 0 ] ; then
        debug "INFO: No difference in the contents of sysfs after deleting the child"
    else 
        grep -qw $newnet /tmp/sys_aftr_child_killed
        if [ $? = 0 ]; then
            debug "INFO: Device $newnet is moved to ParentNS"
        else
            debug "INFO: Device $newnet is moved under diff name in ParentNS"
        fi
    fi
    # Cleanup
    ip link delete $vnet0
    rm -f /tmp/sys_b4_child_killed /tmp/sys_aftr_child_killed /tmp/FIFO6 > /dev/null

