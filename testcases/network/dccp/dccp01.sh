#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2017-2018 Oracle and/or its affiliates. All Rights Reserved.

TST_CNT=3
TST_TESTFUNC="test"
TST_NEEDS_TMPDIR=1
TST_NEEDS_ROOT=1

. tst_net.sh

compare()
{
	local per=$(( $res0 * 100 / $res1 - 100 ))

	if [ "$per" -gt "100" -o "$per" -lt "-100" ]; then
		tst_res TFAIL "$1 performance $per %"
	else
		tst_res TPASS "$1 performance $per % in range -100 ... 100 %"
	fi
}

test1()
{
	tst_res TINFO "run UDP"
	tst_netload -H $(tst_ipaddr rhost) -T udp
	res0="$(cat tst_netload.res)"
}
test2()
{
	tst_res TINFO "compare UDP/DCCP performance"
	tst_netload -H $(tst_ipaddr rhost) -T dccp
	res1="$(cat tst_netload.res)"
	compare DCCP
}
test3()
{
	tst_res TINFO "compare UDP/UDP-Lite performance"
	tst_netload -H $(tst_ipaddr rhost) -T udp_lite
	res1="$(cat tst_netload.res)"
	compare UDP-Lite
}

tst_run
