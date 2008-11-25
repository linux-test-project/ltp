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
## Author:      Veerendra <veeren@linux.vnet.ibm.com>

# This script is pinging the child NS.

TCID=${TCID:-child_2.sh}
TST_TOTAL=1
TST_COUNT=1
export TCID
export TST_COUNT
export TST_TOTAL
. initialize.sh
status=0

    # Writing child PID number into /tmp/FIFO4
    echo $$ > /tmp/FIFO4

    # Reading device name from parent
    vnet2=`cat /tmp/FIFO3`;
    debug "INFO: CHILD2: Network dev name received $vnet2";

    # By now networking is working
    ifconfig $vnet2 $IP4$mask up 2> /dev/null
    ifconfig lo up 

    # Creating ssh session
    /usr/sbin/sshd -p $PORT2
    if [ $? ]; then
        debug "INFO: Started the sshd in CHILD2"
        sshpid2=`ps -ef | grep "sshd -p $PORT2" | awk '{ print $2 ; exit 0} ' `

        ping -qc 2 $IP3 > /dev/null
        if [ $? -ne 0 ] ; then
            tst_resm TFAIL "FAIL: Unable to ping Parent2 from Child2"
            status=-1
        fi
    else
        tst_resm TFAIL "FAIL: Unable to start sshd in child1"
        status=-1
    fi
    # Pinging CHILD1 from CHILD2
    debug "INFO: Trying for pinging CHILD1..."

    ping -qc 2 $IP2 > /dev/null
    if [ $? -eq 0 ]; then
        tst_resm TINFO "PASS: CHILD1 is pinging from CHILD2 ! "
        # Using /tmp/FIFO5 to synchronize with CHILD1
        echo 0 > /tmp/FIFO5
        sleep 2
    else
        tst_resm TFAIL "FAIL: Unable to ping Child1NS from Child2NS !"
        echo 1 > /tmp/FIFO5
        status=-1
    fi

    cleanup $sshpid2 $vnet2
    debug "INFO: ********End of Child-2 $0********"
    exit $status
