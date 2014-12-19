#!/bin/bash

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
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    ##
##                                                                            ##
## Author:      Veerendra <veeren@linux.vnet.ibm.com>                         ##
################################################################################

################################################################################
# This script creates 2 veth devices $vnet0 and $vnet1.
# It will assign $IP1 to $vnet0 .
# And defines the $IP2 as route to $vnet1
# Also it assigns the $vnet1 to child network-NS
#
################################################################################

#set -x

# The test case ID, the test case count and the total number of test case

TCID=${TCID:-netns_paripv6.sh}
TST_TOTAL=1
TST_COUNT=1
export TCID
export TST_COUNT
export TST_TOTAL
. netns_initialize.sh
status=0

    # Sets up the infrastructure for creating network NS
    # By defining the veth pair $vnet0 and $vnet1
    echo 1 > /proc/sys/net/ipv4/ip_forward
    echo 1 > /proc/sys/net/ipv4/conf/$netdev/proxy_arp
    create_veth
    vnet0=$dev0
    vnet1=$dev1
    if [ -z "$vnet0" -o -z "$vnet1" ] ; then
        tst_resm TFAIL "Error: unable to create veth pair"
        exit 1
    else
        debug "INFO: vnet0 = $vnet0 , vnet1 = $vnet1"
    fi

    enable_veth_ipv6 $vnet0
    vnet0_orig_status=$?

    ifconfig $vnet0 $IP1/24 up > /dev/null 2>&1
    route add -host $IP2 dev $vnet0
    echo 1 > /proc/sys/net/ipv4/conf/$vnet0/proxy_arp

    # Waits for the Child-NS to get created and reads the PID
    pid=$(tst_timeout "cat /tmp/FIFO1" $NETNS_TIMEOUT)
    if [ $? -ne 0 ]; then
        tst_brkm TBROK "timeout reached!"
    fi
    debug "INFO: the pid of child is $pid"
    ip link set $vnet1 netns $pid

    tst_timeout "echo $vnet1 > /tmp/FIFO2" $NETNS_TIMEOUT
    if [ $? -ne 0 ]; then
        tst_brkm TBROK "timeout reached!"
    fi

    childIPv6=$(tst_timeout "cat /tmp/FIFO3" $NETNS_TIMEOUT)
    if [ $? -ne 0 ]; then
        tst_brkm TBROK "timeout reached!"
    fi
    debug "INFO: The ipv6 addr of child is $childIPv6"
    # Passes the IPv6 addr to Child NS
    parIPv6=`ip -6 addr show dev $vnet0 | awk ' /inet6/ { print $2 } ' | awk -F"/" ' { print $1 } '`

    tst_timeout "echo $parIPv6 > /tmp/FIFO4" $NETNS_TIMEOUT
    if [ $? -ne 0 ]; then
        tst_brkm TBROK "timeout reached!"
    fi
    ping6 -I $vnet0 -qc 2 $childIPv6 >/dev/null 2>&1

    if [ $? = 0 ] ; then
       tst_resm TINFO "IPv6: Pinging child from parent: PASS"
       status=0
    else
       tst_resm TFAIL "IPv6: Pinging child from parent: FAIL"
       status=-1

    fi
    ret=$(tst_timeout "cat /tmp/FIFO6" $NETNS_TIMEOUT)
    if [ $? -ne 0 ]; then
        tst_brkm TBROK "timeout reached!"
    fi
    if [ -z "$ret" ]; then
        ret="-1"
    fi

    if [ "$ret" != "0" ] ; then
        status=$(expr "$ret")
    fi

    if [ $vnet0_orig_status -eq 1 ];then
       disable_veth_ipv6 $vnet0
    fi
debug "INFO: Done with executing parent script $0 "
exit $status
