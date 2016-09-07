#! /bin/sh
# Copyright (c) 2014-2016 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2000
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it would be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
#  PURPOSE: To test the basic functionality of the `ping` command.
#
#  SETUP: If "RHOST" is not exported, then the local hostname is used.
#
#  HISTORY:
#    06/06/03 Manoj Iyer manjo@mail.utexas.edu
#    - Modified testcase to use test APIs and also fixed minor bugs
#    03/01 Robbie Williamson (robbiew@us.ibm.com)
#      -Ported

TST_TOTAL=10
TCID="ping01"

. test_net.sh

do_setup()
{
	COUNT=${COUNT:-3}
	PACKETSIZES=${PACKETSIZES:-"8 16 32 64 128 256 512 1024 2048 4064"}

	PING_CMD=ping${TST_IPV6}

	tst_check_cmds $PING_CMD
}

do_test()
{
	tst_resm TINFO "$PING_CMD with $PACKETSIZES ICMP packets"
	local ipaddr=$(tst_ipaddr rhost)
	for packetsize in $PACKETSIZES; do
		EXPECT_PASS $PING_CMD -c $COUNT -s $packetsize $ipaddr \>/dev/null
	done
}

do_setup
do_test

tst_exit
