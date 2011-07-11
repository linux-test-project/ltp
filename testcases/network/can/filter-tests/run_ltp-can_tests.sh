#!/bin/sh
################################################################################
## Copyright (c) Oliver Hartkopp <oliver.hartkopp@volkswagen.de>, 2011        ##
## Copyright (c) International Business Machines  Corp., 2009                 ##
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
################################################################################

if [ $(id -ru) -ne 0 ]; then
     echo You need to be root to execute these tests
     exit 1
fi

# load needed CAN networklayer modules
modprobe -f can
modprobe -f can_raw

# ensure the vcan driver to perform the ECHO on driver level
modprobe -r vcan
modprobe -f vcan echo=1

VCAN=vcan0

# create virtual CAN device
ip link add dev $VCAN type vcan || exit 1
ifconfig $VCAN up

# check precondition for CAN frame flow test
HAS_ECHO=`ip link show $VCAN | grep -c ECHO`

if [ $HAS_ECHO -ne 1 ]
then
    exit 1
fi

# test of CAN filters on af_can.c 
./tst-filter $VCAN || exit 1

# test of CAN frame flow down to the netdevice and up again
./tst-rcv-own-msgs $VCAN || exit 1

exit 0


