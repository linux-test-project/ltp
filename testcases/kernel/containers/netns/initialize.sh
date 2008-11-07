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

# This scripts contains the IP addr, PortNum of sshd
# to be used and cleanup functions.
# set -x

TCID=${TCID:-initialize}
TST_TOTAL=1
TST_COUNT=1
export TCID
export TST_COUNT
export TST_TOTAL

IP1='192.168.0.181'
IP2='192.168.0.182'
IP3='192.168.0.183'
IP4='192.168.0.184'
mask=/24
PORT=7890
PORT2=9876
DEBUG=0

    # set the LTPROOT directory
    cd `dirname $0`
    env | grep -q LTPROOT
    if [ $? -eq 0 ]; then
        FS_BIND=${LTPROOT}/testcases/kernel/fs/fs_bind/bin/smount
        if [ -f $FS_BIND ] ; then
            smount=$FS_BIND
        fi
    else
        tst_resm TFAIL "Please set the LTP root env variable, and retry again"
        exit -1
    fi

    IPver=`ip -V | awk  -F"-" ' {  print $2 } '` ;
    if ! printf "%s\n%s\n" "ss080417" "$IPver" | sort -c ; then
        tst_resm  TINFO "ip version should be atleast ss080417"
        exit -1
    fi
    mkfifo /tmp/FIFO1 /tmp/FIFO2 /tmp/FIFO3 /tmp/FIFO4 /tmp/FIFO5 /tmp/FIFO6 2> /dev/null

    netdev=`ip addr show | awk '/^[0-9]*:.*UP/ { a=$2 } /inet / { b=$2 ; \
            if ( a !~ /lo/  && b ! NULL ) {  print a ; exit 0 } } ' `
    netdev=`basename $netdev ":"`
    if [ -z $netdev ] ; then
        tst_resm  TINFO "Not able to determine the ethernet dev name"
        exit -1
    fi

    # copying the values for restoring it later.
    ipfwd=`cat /proc/sys/net/ipv4/ip_forward`
    if [ -f /proc/sys/net/ipv4/conf/$netdev/proxy_arp ] ; then
        arpproxy=`cat /proc/sys/net/ipv4/conf/$netdev/proxy_arp`
    else
	arpproxy=0
    fi
cleanup()
{
   if [ $# == 2 ]; then
        pid=$1
        netdev=$2
    fi

    debug "INFO: doing cleanup operation "
    # Delete the veth pair:
    (ip link delete $netdev) 2> /dev/null
    sleep 1

    #Restoring the orignial values .
    echo $ipfwd > /proc/sys/net/ipv4/ip_forward
    if [ -f /proc/sys/net/ipv4/conf/$netdev/proxy_arp ] ; then
    	echo $arpproxy > /proc/sys/net/ipv4/conf/$netdev/proxy_arp
    fi
    ( kill -s KILL $pid ) 2> /dev/null
    rm -f /tmp/FIFO1 /tmp/FIFO2 /tmp/FIFO3 /tmp/FIFO4 /tmp/FIFO5 /tmp/FIFO6
    rm -f /tmp/net1 /tmp/net2 || true
}

debug()
{
    if [ "$DEBUG" = 1 ]
    then
        echo $1;
    fi
}

create_veth()
{
    ip link show > /tmp/net1
    ip link add type veth
    sleep 1
    ip link show > /tmp/net2
    eval `diff /tmp/net1 /tmp/net2 | awk -F": "  ' /^> [0-9]*:/ { print "dev" i+0 "=" $2; i++  }  '`
}



