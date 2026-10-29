#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019-2022 Petr Vorel <pvorel@suse.cz>

TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="ip"

ROUTE_RHOST_PORT=${ROUTE_RHOST_PORT:-65535}
ROUTE_MAX_IP=${ROUTE_MAX_IP:-5}

IP_ADDR_DELIM=','

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

check_max_ip()
{
	local max_ip_limit=254
	[ "$TST_IPV6" ] && max_ip_limit=65534

	tst_is_int "$ROUTE_MAX_IP" || tst_brk TBROK "\$ROUTE_MAX_IP not int ($ROUTE_MAX_IP)"
	[ $ROUTE_MAX_IP -gt $max_ip_limit ] && ROUTE_MAX_IP=$max_ip_limit
	[ $ROUTE_MAX_IP -gt $ROUTE_CHANGE_NETLINK ] && ROUTE_MAX_IP=$ROUTE_CHANGE_NETLINK
}

cleanup_if()
{
	[ "$new_liface" ] && add_macvlan -d $new_liface
	[ "$new_riface" ] && add_macvlan -d $new_riface rhost
	route_cleanup
}

route_cleanup()
{
	tst_restore_ipaddr
	tst_restore_ipaddr rhost
}

setup_gw()
{
	rt="$(tst_ipaddr_un -p 0 0)"
	lhost="$(tst_ipaddr_un 1 1)"
	rhost="$(tst_ipaddr_un 0 1)"
	tst_add_ipaddr -s -q -a $lhost
	tst_add_ipaddr -s -q -a $rhost rhost
}

setup_if()
{
	rt="$(tst_ipaddr_un -p 0)"
	rhost="$(tst_ipaddr_un 0 1)"
	tst_add_ipaddr -s -q -a $rhost rhost

	if [ $(tst_get_ifaces_cnt) -lt 2 ]; then
		new_liface="ltp_mv2"
		tst_res TINFO "2 or more local ifaces required, adding '$new_liface'"
		add_macvlan $new_liface
	fi

	if [ $(tst_get_ifaces_cnt rhost) -lt 2 ]; then
		new_riface="ltp_mv1"
		tst_res TINFO "2 or more remote ifaces required, adding '$new_riface'"
		add_macvlan $new_riface rhost
	fi
}

test_netlink()
{
	local opt="-c $ROUTE_CHANGE_NETLINK $TST_IPV6_FLAG -p $ROUTE_RHOST_PORT $ROUTE_CHANGE_NETLINK_PARAMS"
	local cmd="route-change-netlink"
	local ret=0

	tst_res TINFO "running $cmd $opt"
	$cmd $opt || ret=$?
	if [ "$ret" -ne 0 ]; then
		if [ $((ret & 3)) -ne 0 ]; then
			tst_res TFAIL "$cmd failed"
			return
		fi

		[ $((ret & 32)) -ne 0 ] && \
			tst_brk TCONF "not supported configuration"

		[ $((ret & 4)) -ne 0 ] && \
			tst_res TWARN "$cmd has warnings"
	fi
	tst_res TPASS "$cmd passed"
}

. tst_net.sh
