#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019-2022 Petr Vorel <pvorel@suse.cz>
# Copyright (c) International Business Machines Corp., 2006
# Author: Mitsuru Chinen <mitch@jp.ibm.com>
# Rewrite into new shell API: Petr Vorel
#
# Change route destination
# lhost: 10.0.0.2, rhost: 10.23.x.1

TST_SETUP="setup"
TST_CLEANUP="route_cleanup"
TST_TESTFUNC="test_dst"

setup()
{
	tst_res TINFO "change IPv$TST_IPVER route destination $ROUTE_CHANGE_IP times"
}

test_dst()
{
	local iface="$(tst_iface)"
	local rt="$(tst_ipaddr_un -p $1)"
	local rhost="$(tst_ipaddr_un $1 1)"

	tst_res TINFO "testing route '$rt'"

	tst_add_ipaddr -s -q -a $rhost rhost
	ROD ip route add $rt dev $iface
	EXPECT_PASS_BRK ping$TST_IPV6 -c1 -I $(tst_ipaddr) $rhost \>/dev/null
	ROD ip route del $rt dev $iface
	tst_del_ipaddr -s -q -a $rhost rhost
}

. route-lib.sh
TST_CNT=$ROUTE_CHANGE_IP
tst_run
