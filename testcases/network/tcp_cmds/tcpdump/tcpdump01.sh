#!/bin/sh
# Copyright (c) 2014 Oracle and/or its affiliates. All Rights Reserved.
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
# along with this program; if not, write the Free Software Foundation,
# Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
#  PURPOSE: To test the basic functionality of `tcpdump`.
#
#  HISTORY:
#    04/17/01 Robbie Williamson (robbiew@us.ibm.com)
#      -Written

TST_TOTAL=1
TCID="tcpdump01"
TST_CLEANUP=do_cleanup
TST_USE_LEGACY_API=1

do_setup()
{
	ping_cmd=ping$TST_IPV6
	tst_require_cmds tcpdump kill $ping_cmd
	outfile="tcpdump_out"
	numloops=20
	tst_tmpdir
}

do_test()
{
	addr=$(tst_ipaddr rhost)
	tst_resm TINFO "start $ping_cmd in background"

	$ping_cmd -I $(tst_iface) -f $addr > /dev/null 2>&1 &
	ping_pid=$!

	tst_resm TINFO "running tcpdump..."
	tcpdump -n -i $(tst_iface) -c $numloops > $outfile 2>/dev/null

	[ $? -ne 0 ] && tst_brkm TBROK "problems trying to launch tcpdump"

	grep "$addr\>" $outfile > /dev/null 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "'$addr' was not listed in network traffic"
		return
	fi

	tst_resm TPASS "Test finished successfully"
}

do_cleanup()
{
	kill $ping_pid > /dev/null 2>&1
	wait $ping_pid > /dev/null 2>&1
	tst_rmdir
}

. tst_net.sh

do_setup
do_test
tst_exit
