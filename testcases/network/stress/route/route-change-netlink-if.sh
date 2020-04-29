#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
#
# Change route interface
# gw (on rhost): 10.23.x.1, rhost: 10.23.0.1, switching ifaces on lhost
# lhost (iface set, but not specified in Netlink API): 10.23.x.2

TST_SETUP="setup"
TST_CLEANUP="cleanup_if"
TST_TESTFUNC="test_netlink"
. route-lib.sh

setup()
{
	local gw gw_all iface iface_all
	local cnt=0

	tst_res TINFO "change IPv$TST_IPVER route interface $ROUTE_CHANGE_NETLINK times"
	setup_if

	while [ $cnt -lt $(tst_get_ifaces_cnt) ]; do
		gw="$(tst_ipaddr_un -n1 $cnt 1)"
		iface="$(tst_iface lhost $cnt)"
		lhost="$(tst_ipaddr_un -n1 $cnt 2)"

		tst_add_ipaddr -s -q -a $lhost lhost $cnt
		tst_add_ipaddr -s -q -a $gw rhost $cnt

		gw_all="$gw$IP_ADDR_DELIM$gw_all"
		iface_all="$iface$IP_ADDR_DELIM$iface_all"

		cnt=$((cnt+1))
	done

	ROUTE_CHANGE_NETLINK_PARAMS="-d '$iface_all' -g '$gw_all' -r $rhost"
}

tst_run
