#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>
# Copyright (c) International Business Machines Corp., 2006
# Author: Mitsuru Chinen <mitch@jp.ibm.com>
# Rewrite into new shell API: Petr Vorel
#
# Change route gateway
# lhost: 10.23.1.1, gw (on rhost): 10.23.1.x, rhost: 10.23.0.1

TST_TESTFUNC="test_gw"
. route-lib.sh

setup()
{
	tst_res TINFO "change IPv$TST_IPVER route gateway $NS_TIMES times"

	rt="$(tst_ipaddr_un -p 0 0)"
	lhost="$(tst_ipaddr_un 1 1)"
	rhost="$(tst_ipaddr_un 0 1)"
	tst_add_ipaddr -s -q -a $lhost
	tst_add_ipaddr -s -q -a $rhost rhost
}

test_gw()
{
	local gw="$(tst_ipaddr_un -h 2,254 1 $(($1 + 1)))"
	local iface="$(tst_iface)"

	tst_res TINFO "testing route over gateway '$gw'"

	tst_add_ipaddr -s -q -a $gw rhost
	ROD ip route add $rt dev $iface via $gw
	EXPECT_PASS_BRK ping$TST_IPV6 -c1 -I $lhost $rhost \>/dev/null
	ROD ip route del $rt dev $iface via $gw
	tst_del_ipaddr -s -q -a $gw rhost
}

tst_run
