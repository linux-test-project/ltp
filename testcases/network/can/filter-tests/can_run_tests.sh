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
## along with this program;  if not, write to the Free Software Foundation,   ##
## Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA           ##
##                                                                            ##
################################################################################

TCID="$1"
TST_TOTAL=1
export TST_COUNT

. test.sh

setup()
{
	tst_require_root

	# load needed CAN networklayer modules
	modprobe can
	ret=$?
	if [ $ret -ne 0 ]; then
		tst_brkm TCONF "modprobe can failed: ret - $ret"
	fi

	modprobe can_raw
	ret=$?
	if [ $ret -ne 0 ]; then
		tst_brkm TCONF "modprobe can_raw failed: ret - $ret"
	fi

	# ensure the vcan driver to perform the ECHO on driver level
	modprobe -r vcan
	ret=$?
	if [ $ret -ne 0 ]; then
		tst_brkm TCONF "modprobe -r vcan failed: ret - $ret"
	fi

	modprobe vcan echo=1
	ret=$?
	if [ $ret -ne 0 ]; then
		tst_brkm TCONF "modprobe vcan echo=1 failed: ret - $ret"
	fi

	VCAN=vcan0

	# create virtual CAN device
	ip link add dev $VCAN type vcan
	ret=$?
	if [ $ret -ne 0 ]; then
		tst_brkm TBROK \
			 "ip link add dev $VCAN type vcan failed: ret - $ret"
	fi

	ip link set dev $VCAN up
	ret=$?
	if [ $ret -ne 0 ]; then
		tst_brkm TBROK "ip link set dev $VCAN up failed: ret - $ret"
	fi

	# check precondition for CAN frame flow test
	HAS_ECHO=`ip link show $VCAN | grep -c ECHO`
	if [ $HAS_ECHO -ne 1 ]; then
		tst_brkm TBROK "ECHO is not 1"
	fi
}

cleanup()
{
	ip link set dev $VCAN down
	ip link del dev $VCAN
	modprobe -r vcan
	modprobe -r can_raw
	modprobe -r can
}

if [ $# -ne 1 ]; then
	tst_brkm TBROK "Usage: $0 [can_filter | can_rcv_own_msgs]"
fi

setup
TST_CLEANUP=cleanup

"$1" "$VCAN"
ret=$?
case "$ret" in
0)	tst_resm TPASS "Test $1 PASS";;
1)	tst_resm TFAIL "Test $1 FAIL";;
32)	tst_resm TCONF "$1 is not appropriate for configuration flag";;
*)	tst_resm TBROK "Invalid resm type $ret";;
esac

tst_exit
