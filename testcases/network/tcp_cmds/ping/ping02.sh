#! /bin/sh
# Copyright (c) 2016 Oracle and/or its affiliates. All Rights Reserved.
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

TST_TOTAL=10
TCID="ping02"

. test_net.sh

do_setup()
{
	COUNT=${COUNT:-3}
	PACKETSIZES=${PACKETSIZES:-"8 16 32 64 128 256 512 1024 2048 4064"}

	PING=ping${TST_IPV6}

	tst_check_cmds $PING
}

do_test()
{
	local pat="000102030405060708090a0b0c0d0e0f"

	tst_resm TINFO "flood $PING: ICMP packets filled with pattern '$pat'"

	local ipaddr=$(tst_ipaddr rhost)
	for psize in $PACKETSIZES; do
		EXPECT_PASS $PING -c $COUNT -f -s $psize $ipaddr -p "$pat" \>/dev/null
	done
}

do_setup
do_test

tst_exit
