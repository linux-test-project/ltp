#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
#
# Change route gateway via netlink
# gw (on rhost): 10.23.1.x, rhost: 10.23.0.1
# lhost (iface set, but not specified in Netlink API): 10.23.1.1

TST_SETUP="setup"
TST_CLEANUP="route_cleanup"
TST_TESTFUNC="test_netlink"
. route-lib.sh

setup()
{
	local cnt=0

	tst_res TINFO "change IPv$TST_IPVER route gateway $ROUTE_CHANGE_NETLINK times"

	check_max_ip
	setup_gw

	while [ $cnt -lt $ROUTE_MAX_IP ]; do
		gw="$(tst_ipaddr_un -h 2,$max_ip_limit 1 $(($cnt + 1)))"
		gw_all="$gw$IP_ADDR_DELIM$gw_all"
		tst_add_ipaddr -s -q -a $gw rhost
		cnt=$((cnt+1))
	done

	ROUTE_CHANGE_NETLINK_PARAMS="-d $(tst_iface) -g "$gw_all" -r $rhost"
}

tst_run
