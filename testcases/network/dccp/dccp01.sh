#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2017-2020 Oracle and/or its affiliates. All Rights Reserved.

TST_CNT=3
TST_TESTFUNC="test"
TST_NEEDS_TMPDIR=1
TST_NEEDS_ROOT=1


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
	tst_netload_compare $res0 $res1 -100 100
}
test3()
{
	tst_res TINFO "compare UDP/UDP-Lite performance"
	tst_netload -H $(tst_ipaddr rhost) -T udp_lite
	res1="$(cat tst_netload.res)"
	tst_netload_compare $res0 $res1 -100 100
}

. tst_net.sh
tst_run
