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
# This test scripts passes the PID of the child NS to the parent NS.
# Also it assigns the device address and starts the sshd daemon
# It checks for basic network connection between parent and child.
# It renames the network device of the child 
#
# Arguments:    Accepts an argument 'script' and executes on top of this
################################################################################
#set -x
# The test case ID, the test case count and the total number of test case

TCID=${TCID:-childipv6.sh}
TST_TOTAL=1
TST_COUNT=1
export TCID
export TST_COUNT
export TST_TOTAL
.  initialize.sh
status=0
    
    # Passing the PID of child 
    echo $$ > /tmp/FIFO1
    
    # waiting for the virt-eth devname and IPv6 addr from parent
    vnet1=`cat /tmp/FIFO2`
    # Assigning the dev addresses
    ifconfig $vnet1 $IP2/24 up > /dev/null 2>&1
    ifconfig lo up
    sleep 2
    
    #starting the sshd inside the child NS
    /usr/sbin/sshd -p $PORT 
    if [ $? = 0 ]; then
        debug "INFO: started the sshd @ port no $PORT"
        sshpid=`ps -ef | grep "sshd -p $PORT" | awk '{ print $2 ; exit 0} ' `
    else
        tst_resm TFAIL "Failed in starting ssh @ port $PORT"
        status=1
    fi
    
    childIPv6=`ip -6 addr show dev $vnet1 | awk ' /inet6/ { print $2 } ' | awk -F"/" ' { print $1 } '`
    echo $childIPv6 >> /tmp/FIFO3
    
    parIPv6=`cat /tmp/FIFO4`
    debug "INFO: Received the Ipv6 addr $parIPv6"

    # checking if parent ns responding
    ping6 -I $vnet1 -qc 2 $parIPv6 >/dev/null 2>&1 
           if [ $? = 0 ] ; then
               tst_resm TINFO "IPv6: Pinging Parent from Child: PASS"
            else
               tst_resm TFAIL "IPv6: Pinging Parent from Child: FAIL"
               status=1
            fi
    echo $status > /tmp/FIFO6
    debug "INFO: Done with executing child script $0"
    cleanup $sshpid $vnet1
    exit $status
