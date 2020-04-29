#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019-2020 Petr Vorel <pvorel@suse.cz>
# Copyright (c) International Business Machines Corp., 2006
# Author: Mitsuru Chinen <mitch@jp.ibm.com>
# Rewrite into new shell API: Petr Vorel
#
# Change route interface
# lhost: 10.23.x.2, gw (on rhost): 10.23.x.1, rhost: 10.23.0.1, switching ifaces on lhost

TST_SETUP="setup"
TST_CLEANUP="cleanup_if"
TST_TESTFUNC="test_if"
. route-lib.sh
TST_CNT=$ROUTE_CHANGE_IP

setup()
{
	tst_res TINFO "change IPv$TST_IPVER route interface $ROUTE_CHANGE_IP times"
	setup_if
}

test_if()
{
	local gw="$(tst_ipaddr_un -n1 $1 1)"
	local lhost="$(tst_ipaddr_un -n1 $1 2)"
	local link_num="$(($1 % $(tst_get_ifaces_cnt)))"
	local iface="$(tst_iface lhost $link_num)"

	tst_res TINFO "testing route over interface '$iface' with gateway '$gw'"

	tst_add_ipaddr -s -q -a $lhost lhost $link_num
	tst_add_ipaddr -s -q -a $gw rhost $link_num
	ROD ip route add $rt dev $iface via $gw
	EXPECT_PASS_BRK ping$TST_IPV6 -c1 -I $lhost $rhost \>/dev/null
	ROD ip route del $rt dev $iface via $gw
	tst_del_ipaddr -s -q -a $lhost lhost $link_num
	tst_del_ipaddr -s -q -a $gw rhost $link_num
}

tst_run
