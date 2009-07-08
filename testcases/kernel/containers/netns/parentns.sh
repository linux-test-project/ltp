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
# This script creates 2 veth devices.
# It will assign $IP1 to vnet0 .
# And defines the $IP2 as route to vnet1
# Also it assigns the $vnet1 to child network-NS
#
# Arguments:    Accepts an argument 'script' and executes on top of this
################################################################################

# The test case ID, the test case count and the total number of test case

TCID=${TCID:-parentns.sh}
TST_TOTAL=1
TST_COUNT=1
export TCID
export TST_COUNT
export TST_TOTAL
#set -x
. initialize.sh
status=0

    # Checks if any script is passed as argument.
    if [ $# = 2 ]; then
        scrpt=$1
        debug "INFO: Script to be executed in parent NS is $scrpt"
    fi

    # Sets up the infrastructure for creating network NS

    echo 1 > /proc/sys/net/ipv4/ip_forward
    echo 1 > /proc/sys/net/ipv4/conf/$netdev/proxy_arp

    create_veth
    vnet0=$dev0
    vnet1=$dev1
    if [ -z "$vnet0" -o -z "$vnet1" ] ; then
        tst_resm TFAIL  "Error: unable to create veth pair"
        exit -1
    else
        debug "INFO: vnet0 = $vnet0 , vnet1 = $vnet1"
    fi
    sleep 2

    ifconfig $vnet0 $IP1/24 up > /dev/null 2>&1
	if [ $? -ne 0 ]; then
		debug "Failed to make interface $vnet0 up in parent....."
	fi
    route add -host $IP2 dev $vnet0
	if [ $? -ne 0 ]; then
		debug "Failed to add route to child in parent for $vnet0....."
	fi
    echo 1 > /proc/sys/net/ipv4/conf/$vnet0/proxy_arp

    # Waits for the Child-NS to get created and reads the PID
    tmp=`cat /tmp/FIFO1`;
    pid=$2;
    debug "INFO: the pid of child is $pid"
    ip link set $vnet1 netns $pid
    if [ $? -ne 0 ]; then
	echo "Failed to assign network device to child .........."
    fi

    # Passes the device name to Child NS
    echo $vnet1 > /tmp/FIFO2

    # Executes the script if it is passed as an argument.
    if [ ! -z $scrpt ] && [ -f $scrpt ] ;  then
        . $scrpt
    fi

    debug "INFO: Done executing parent script $0, status is $status "
    exit $status

