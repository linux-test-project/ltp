#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>
# Copyright (c) International Business Machines Corp., 2006
# Author: Mitsuru Chinen <mitch@jp.ibm.com>
# Rewrite into new shell API: Petr Vorel
#
# Change route interface
# lhost: 10.23.x.2, gw (on rhost): 10.23.x.1, rhost: 10.23.0.1, switching ifaces on lhost

TST_TESTFUNC="test_if"
. route-lib.sh
TST_CLEANUP="cleanup"

add_macvlan()
{
	local action="add"
	local OPTIND
	while getopts d opt; do
		case "$opt" in
		d) action="del";;
		esac
	done
	shift $((OPTIND - 1))

	local iface="$1"
	local type="${2:-lhost}"

	cmd="ip link $action $iface link $(tst_iface $type) type macvlan mode bridge"
	if [ $type = "lhost" ]; then
		ROD $cmd
		[ "$action" = "add" ] || return
		LHOST_IFACES="$LHOST_IFACES $iface"
	else
		tst_rhost_run -s -c "$cmd"
		[ "$action" = "add" ] || return
		RHOST_IFACES="$RHOST_IFACES $iface"
	fi
	tst_init_iface $type 1
}

setup()
{
	tst_res TINFO "change IPv$TST_IPVER route interface $NS_TIMES times"

	rt="$(tst_ipaddr_un -p 0)"
	rhost="$(tst_ipaddr_un 0 1)"
	tst_add_ipaddr -s -q -a $rhost rhost

	if [ $(tst_get_ifaces_cnt) -lt 2 ]; then
		new_liface="ltp_mv2"
		tst_res TINFO "2 or more local ifaces required, adding $new_liface"
		add_macvlan $new_liface
	fi

	if [ $(tst_get_ifaces_cnt rhost) -lt 2 ]; then
		new_riface="ltp_mv1"
		tst_res TINFO "2 or more remote ifaces required, adding $new_riface"
		add_macvlan $new_riface rhost
	fi
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

cleanup()
{
	[ "$new_liface" ] && add_macvlan -d $new_liface
	[ "$new_riface" ] && add_macvlan -d $new_riface rhost
	route_cleanup
}

tst_run
