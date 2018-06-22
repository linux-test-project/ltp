#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2016-2018 Oracle and/or its affiliates.
#
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>

TST_TOTAL=2
TCID="busy_poll03"
TST_NEEDS_TMPDIR=1

TST_USE_LEGACY_API=1
. tst_net.sh
. busy_poll_lib.sh

cleanup()
{
	tst_rmdir

	sysctl -q -w net.core.busy_poll=$busy_poll_old
	tst_rhost_run -c "sysctl -q -w net.core.busy_poll=$rbusy_poll_old"
}

set_busy_poll()
{
	local value=${1:-"0"}
	ROD_SILENT sysctl -q -w net.core.busy_poll=$value
	tst_rhost_run -s -c "sysctl -q -w net.core.busy_poll=$value"
}

busy_poll_old="$(cat /proc/sys/net/core/busy_poll)"
rbusy_poll_old=$(tst_rhost_run -c 'cat /proc/sys/net/core/busy_poll')

TST_CLEANUP="cleanup"
trap "tst_brkm TBROK 'test interrupted'" INT

do_test()
{
	for x in 50 0; do
		tst_resm TINFO "set low latency busy poll to $x per $1 socket"
		set_busy_poll $x
		tst_netload -H $(tst_ipaddr rhost) -n 10 -N 10 -d res_$x \
			    -b $x -T $1
	done

	poll_cmp=$(( 100 - ($(cat res_50) * 100) / $(cat res_0) ))

	if [ "$poll_cmp" -lt 1 ]; then
		tst_resm TFAIL "busy poll result is '$poll_cmp' %"
	else
		tst_resm TPASS "busy poll increased performance by '$poll_cmp' %"
	fi
}

do_test udp
do_test udp_lite

tst_exit
