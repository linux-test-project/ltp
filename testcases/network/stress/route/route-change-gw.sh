#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019-2020 Petr Vorel <pvorel@suse.cz>
# Copyright (c) International Business Machines Corp., 2006
# Author: Mitsuru Chinen <mitch@jp.ibm.com>
# Rewrite into new shell API: Petr Vorel
#
# Change route gateway
# lhost: 10.23.1.1, gw (on rhost): 10.23.1.x, rhost: 10.23.0.1

TST_SETUP="setup"
TST_CLEANUP="route_cleanup"
TST_TESTFUNC="test_gw"
. route-lib.sh
TST_CNT=$ROUTE_CHANGE_IP

setup()
{
	tst_res TINFO "change IPv$TST_IPVER route gateway $ROUTE_CHANGE_IP times"
	setup_gw
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
