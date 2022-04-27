#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2020-2022 Petr Vorel <pvorel@suse.cz>
#
# Change route destination via netlink
# rhost: 10.23.x.1
# lhost (iface set, but not specified in Netlink API): 10.0.0.2

TST_SETUP="setup"
TST_CLEANUP="route_cleanup"
TST_TESTFUNC="test_netlink"

setup()
{
	tst_res TINFO "change IPv$TST_IPVER route destination $ROUTE_CHANGE_NETLINK times"

	local cnt=0
	local gw rhost rhost_all rt

	check_max_ip

	while [ $cnt -lt $ROUTE_MAX_IP ]; do
		rt="$(tst_ipaddr_un -p $cnt)"
		rhost="$(tst_ipaddr_un $cnt 1)"
		rhost_all="$rhost$IP_ADDR_DELIM$rhost_all"

		tst_add_ipaddr -s -q -a $rhost rhost
		ROD ip route add $rt dev $(tst_iface)
		cnt=$((cnt+1))
	done

	ROUTE_CHANGE_NETLINK_PARAMS="-d $(tst_iface) -r '$rhost_all'"
}

. route-lib.sh
tst_run
