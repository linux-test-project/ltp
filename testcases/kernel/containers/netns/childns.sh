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


################################################################################
# This test scripts passes the PID of the child NS to the parent NS.
# Also it assigns the device address and starts the sshd daemon
# It checks for basic network connection between parent and child.
#
# Arguments:    Accepts an argument 'script' and executes on top of this
################################################################################
# set -x
# The test case ID, the test case count and the total number of test case

export TCID=${TCID:-childns.sh}
. cmdlib.sh
exists awk grep ip ping sshd
. initialize.sh
status=0
    
SSHD=`which sshd`

if [ $# -eq 1 ] ; then
    childscrpt=$1
    debug "INFO: The script to be executed in child NS is $childscrpt"
fi

# Passing the PID of child 
echo "child ready" > /tmp/FIFO1;

# waiting for the device name from parent
vnet1=`cat /tmp/FIFO2`;
debug "INFO: network dev name received $vnet1";
# Assigning the dev addresses
if ! ifconfig $vnet1 $IP2/24 up > /dev/null 2>&1 ; then
    debug "Failed to make interface $vnet1 up in child....."
fi

ifconfig lo up
sleep 2

#starting the sshd inside the child NS
if $SSHD -p $PORT; then
    debug "INFO: started the sshd @ port no $PORT"
    sshpid=`ps -ef | grep "sshd -p $PORT" | grep -v grep | awk '{ print $2 ; exit 0} ' `
else
    tst_resm TFAIL "Failed in starting ssh @ port $PORT"
    cleanup $vnet1
    status=1
fi

if [ $status -eq 0 ] ; then

    # checking if parent ns responding
    if ! ping -q -c 2 $IP1 > /dev/null ; then
        tst_resm TFAIL "FAIL: ParentNS not responding"
        status=1
        cleanup $sshpid $vnet1
        exit $status
    fi

    if [ -f "$childscrpt" ]; then
        . "$childscrpt"
    fi

    cleanup $sshpid $vnet1
    debug "INFO: Done with executing child script $0, status is $status"

fi

exit $status 
