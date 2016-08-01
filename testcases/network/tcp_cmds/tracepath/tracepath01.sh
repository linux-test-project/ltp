#!/bin/sh
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
#
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>

TCID=tracepath01
TST_TOTAL=1
. test_net.sh

test_tracepath()
{
	local cmd="$1"
	local len=1280
	local output=
	local ret=0
	local rhost="$2"
	tst_check_cmds "$cmd"

	tst_resm TINFO "test $cmd with $rhost, pmtu is $len"

	output=$($cmd $rhost -l $len | grep "pmtu $len")
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "$cmd failed: pmtu $len not found in output"
		return
	fi

	# Usually only one hop is required to get to remote test machine
	hops_num=$(echo "$output" | sed -nE 's/.*hops ([0-9]+).*/\1/p')
	if [ -z "$hops_num" ]; then
		tst_resm TFAIL "failed to trace path to '$rhost'"
		return
	fi

	if [ "$hops_num" -eq 0 ]; then
		tst_resm TFAIL "can't trace path to '$rhost' in 1+ hops"
		return
	fi

	tst_resm TPASS "traced path to '$rhost' in $hops_num hops"
}

test_tracepath tracepath$TST_IPV6 $(tst_ipaddr rhost)

tst_exit
