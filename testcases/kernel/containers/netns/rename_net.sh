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

# This script Renames the net device of the child ns to $NewNetDev.

# set -x
TCID=${TCID:-rename_net.sh}
TST_TOTAL=1
TST_COUNT=1
export TCID
export TST_COUNT
export TST_TOTAL

    # Find the free dev name 
    for i in `seq 1 100`
    do
        newdev=veth$i
        ip link show | grep -qw $newdev
        # On finding free device break.
        if [ $? != 0 ] ; then 
                break
        fi
    done

    ifconfig $vnet1 down
    ip link set $vnet1 name $newdev
    ifconfig $newdev $IP2/24 up > /dev/null 2>&1

    if [ $? = 0 ] ; then
        tst_resm TINFO "Successfully Renamed device to $newdev"
        if [ "$DEBUG" = 1 ]; then
                ifconfig
        fi
    else
        tst_resm TFAIL "Renaming of device failed: FAIL"
        status=-1
    fi

    if [ $status = 0 ] ; then
        echo $sshpid > /tmp/FIFO3
        echo $newdev > /tmp/FIFO4
    else
        echo FAIL > /tmp/FIFO3
        echo -1 > /tmp/FIFO4
    fi
