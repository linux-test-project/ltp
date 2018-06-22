#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2016-2018 Oracle and/or its affiliates.
#
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>

TST_TEST_DATA="udp udp_lite"

. busy_poll_lib.sh

cleanup()
{
	[ -n "$busy_poll_old" ] && \
		sysctl -q -w net.core.busy_poll=$busy_poll_old
	[ -n "$rbusy_poll_old" ] && \
		tst_rhost_run -c "sysctl -q -w net.core.busy_poll=$rbusy_poll_old"
}

set_busy_poll()
{
	local value=${1:-"0"}
	ROD_SILENT sysctl -q -w net.core.busy_poll=$value
	tst_rhost_run -s -c "sysctl -q -w net.core.busy_poll=$value"
}

setup()
{
	busy_poll_check_config

	busy_poll_old="$(cat /proc/sys/net/core/busy_poll)"
	rbusy_poll_old=$(tst_rhost_run -c 'cat /proc/sys/net/core/busy_poll')
}

test()
{
	for x in 50 0; do
		tst_res TINFO "set low latency busy poll to $x per $2 socket"
		set_busy_poll $x
		tst_netload -H $(tst_ipaddr rhost) -n 10 -N 10 -d res_$x \
			    -b $x -T $2
	done

	local poll_cmp=$(( 100 - ($(cat res_50) * 100) / $(cat res_0) ))

	if [ "$poll_cmp" -lt 1 ]; then
		tst_res TFAIL "busy poll result is '$poll_cmp' %"
	else
		tst_res TPASS "busy poll increased performance by '$poll_cmp' %"
	fi
}

tst_run
