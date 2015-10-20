#!/bin/sh
# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
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

TCID=dctcp01
TST_TOTAL=1
TST_CLEANUP="cleanup"
def_alg="cubic"
prev_alg=

. test_net.sh

set_cong_alg()
{
	local alg=$1
	tst_resm TINFO "setting $alg"

	ROD sysctl -q -w net.ipv4.tcp_congestion_control=$alg
	tst_rhost_run -s -c "sysctl -q -w net.ipv4.tcp_congestion_control=$alg"
}

cleanup()
{
	if [ "$prev_cong_ctl" ]; then
		sysctl -q -w net.ipv4.tcp_congestion_control=$prev_alg
		tst_rhost_run -c \
			"sysctl -q -w net.ipv4.tcp_congestion_control=$prev_alg"
	fi
	tst_rmdir
	tc qdisc del dev $(tst_iface) root netem loss 0.03% ecn
}

setup()
{
	tst_kvercmp 3 18 0
	[ $? -eq 0 ] && tst_brkm TCONF "test requires kernel 3.18 or newer"

	tst_require_root
	tst_check_cmds ip sysctl tc

	tst_resm TINFO "emulate congestion with packet loss 0.03% and ECN"
	tc qdisc add dev $(tst_iface) root netem loss 0.03% ecn > /dev/null 2>&1
	if [ $? -ne 0 ]; then
		tst_brkm TCONF "netem doesn't support ECN"
	fi

	tst_tmpdir

	prev_alg="$(cat /proc/sys/net/ipv4/tcp_congestion_control)"
}

test_run()
{
	tst_resm TINFO "compare '$def_alg' and 'dctcp' congestion alg. results"

	set_cong_alg "$def_alg"

	tst_netload $(tst_ipaddr rhost) tfo_res TFO
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "test with '$def_alg' has failed"
		return
	fi
	local res0="$(cat tfo_res)"
	tst_resm TINFO "$def_alg time '$res0' ms"

	set_cong_alg "dctcp"

	tst_netload $(tst_ipaddr rhost) tfo_res TFO
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "test with 'dctcp' has failed"
		return
	fi
	local res1="$(cat tfo_res)"
	tst_resm TINFO "dctcp time '$res1' ms"

	local per=$(( $res0 * 100 / $res1 - 100 ))

	if [ "$per" -lt "10" ]; then
		tst_resm TFAIL "dctcp performance $per %"
	else
		tst_resm TPASS "dctcp performance $per %"
	fi
}

setup

test_run

tst_exit
