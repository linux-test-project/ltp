#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 Oracle and/or its affiliates. All Rights Reserved.

TST_NEEDS_TMPDIR=1
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="sysctl tc"

. tst_net.sh

def_alg="cubic"
prev_alg=

set_cong_alg()
{
	local alg=$1
	tst_res TINFO "setting $alg"

	tst_set_sysctl net.ipv4.tcp_congestion_control $alg safe
}

tcp_cc_cleanup()
{
	[ "$prev_cong_ctl" ] && \
		tst_set_sysctl net.ipv4.tcp_congestion_control $prev_alg
}

tcp_cc_setup()
{
	prev_alg="$(sysctl -n net.ipv4.tcp_congestion_control)"
}

tcp_cc_test01()
{
	local alg=$1
	local threshold=${2:-10}

	tst_res TINFO "compare '$def_alg' and '$alg' congestion alg. results"

	set_cong_alg "$def_alg"

	tst_netload -H $(tst_ipaddr rhost) -A 15000
	local res0="$(cat tst_netload.res)"

	set_cong_alg "$alg"

	tst_netload -H $(tst_ipaddr rhost) -A 15000
	local res1="$(cat tst_netload.res)"

	local per=$(( $res0 * 100 / $res1 - 100 ))

	if [ "$per" -lt "$threshold" ]; then
		tst_res TFAIL "$alg performance $per %"
	else
		tst_res TPASS "$alg performance $per %"
	fi
}
