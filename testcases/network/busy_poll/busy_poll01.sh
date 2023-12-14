#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2015-2020 Oracle and/or its affiliates. All Rights Reserved.
#
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>

cleanup()
{
	[ -n "$busy_read_old" ] && \
		sysctl -q -w net.core.busy_read=$busy_read_old
	[ -n "$busy_poll_old" ] && \
		sysctl -q -w net.core.busy_poll=$busy_poll_old
	[ -n "$rbusy_read_old" ] && \
		tst_rhost_run -c "sysctl -q -w net.core.busy_read=$rbusy_read_old"
	[ -n "$rbusy_poll_old" ] && \
		tst_rhost_run -c "sysctl -q -w net.core.busy_poll=$rbusy_poll_old"
}

set_busy_poll()
{
	local value=${1:-"0"}
	tst_set_sysctl net.core.busy_read $value safe
	tst_set_sysctl net.core.busy_poll $value safe
}

setup()
{
	busy_poll_check_config

	busy_read_old="$(sysctl -n net.core.busy_read)"
	busy_poll_old="$(sysctl -n net.core.busy_poll)"

	rbusy_read_old=$(tst_rhost_run -c 'sysctl -ne net.core.busy_read')
	rbusy_poll_old=$(tst_rhost_run -c 'sysctl -ne net.core.busy_poll')
}

test()
{
	for x in 50 0; do
		tst_res TINFO "set low latency busy poll to $x"
		set_busy_poll $x
		tst_netload -H $(tst_ipaddr rhost) -n 10 -N 10 -f res_$x
	done

	tst_netload_compare $(cat res_0) $(cat res_50) 1
}

. busy_poll_lib.sh
tst_run
