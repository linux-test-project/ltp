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

# This testcase creates the net devices

# The test case ID, the test case count and the total number of test case

TCID=${TCID:-parent_1.sh}
TST_TOTAL=1
TST_COUNT=1
export TCID
export TST_COUNT
export TST_TOTAL

    . initialize.sh
    echo 1 > /proc/sys/net/ipv4/ip_forward
    echo 1 > /proc/sys/net/ipv4/conf/$netdev/proxy_arp
    create_veth
    vnet0=$dev0
    vnet1=$dev1
    if [ -z "$vnet0" -o -z "$vnet1" ] ; then
        tst_resm TFAIL  "Error: unable to create veth pair in $0"
        exit -1
    else
        debug "INFO: vnet0 = $vnet0 , vnet1 = $vnet1"
    fi

    ifconfig $vnet0 $IP1$mask up > /dev/null 2>&1
    route add -host $IP2 dev $vnet0
    echo 1 > /proc/sys/net/ipv4/conf/$vnet0/proxy_arp

    pid=`cat /tmp/FIFO2` 
    debug "INFO: The pid of CHILD1 is $pid"
    ip link set $vnet1 netns $pid
    echo $vnet1 > /tmp/FIFO1

    debug "INFO: PARENT_1: End of $0"
    exit 0
