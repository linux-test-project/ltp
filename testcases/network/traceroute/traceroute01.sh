#!/bin/sh
# Copyright (c) 2017 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2001
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

TST_TOTAL=6
TCID="traceroute01"
TST_CLEANUP="cleanup"

. test_net.sh

cleanup()
{
	tst_rmdir
}

setup()
{
	tst_resm TINFO "traceroute version:"
	tst_resm TINFO $(traceroute --version 2>&1)

	tst_check_cmds traceroute
	tst_tmpdir
}

run_trace()
{
	local opts="$@"
	local ip=$(tst_ipaddr rhost)

	# According to man pages, default sizes:
	local bytes=60
	[ "$TST_IPV6" ] && bytes=80

	EXPECT_PASS traceroute $ip $bytes -n -m 2 $opts \>out.log 2>&1

	grep -q "$bytes byte" out.log
	if [ $? -ne 0 ]; then
		cat out.log
		tst_resm TFAIL "'$bytes byte' not found"
	else
		tst_resm TPASS "traceroute use $bytes bytes"
	fi

	tail -1 out.log | grep -qE "^[ ]+1[ ]+$ip([ ]+[0-9]+[.][0-9]+ ms){3}"
	if [ $? -ne 0 ]; then
		cat out.log
		tst_resm TFAIL "^[ ]+1[ ]+$ip([ ]+[0-9]+[.][0-9]+ ms){3}' "
			"pattern not found in log"
	else
		tst_resm TPASS "traceroute test completed with 1 hop"
	fi
}

setup
tst_resm TINFO "run traceroute with ICMP ECHO"
run_trace -I
tst_resm TINFO "run traceroute with TCP SYN"
run_trace -T

tst_exit
