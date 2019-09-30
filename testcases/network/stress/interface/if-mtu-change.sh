#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2017-2019 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2015-2017 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2005
# Author: Mitsuru Chinen <mitch@jp.ibm.com>

IF_CMD='ifconfig'
TST_SETUP="do_setup"
TST_CLEANUP="do_cleanup"
. if-lib.sh

# The interval of the mtu change [second]
CHANGE_INTERVAL=${CHANGE_INTERVAL:-5}

TST_TIMEOUT=$(((CHANGE_INTERVAL + 30) * MTU_CHANGE_TIMES))

# The array of the value which MTU is changed into sequentially
# 552 - net.ipv4.route.min_pmtu
CHANGE_VALUES="784 1142 552 1500 552 1500 552 748 552 1142 1500"
CHANGE6_VALUES="1280 1445 1335 1390 1500 1280 1500 1280 1335 1500"
saved_mtu=

do_setup()
{
	[ "$TST_IPV6" ] && CHANGE_VALUES=$CHANGE6_VALUES
	if_setup
	saved_mtu="$(cat /sys/class/net/$(tst_iface)/mtu)"
}

do_cleanup()
{
	if_cleanup_restore
	if [ "$saved_mtu" ]; then
		ip li set $(tst_iface) mtu $saved_mtu
		tst_rhost_run -c "ip li set $(tst_iface rhost) mtu $saved_mtu"
	fi
}

test_body()
{
	local cmd="$CMD"

	local iface=$(tst_iface)
	local iface_rmt=$(tst_iface rhost)
	[ "$TST_IPV6" ] && local netmask=64 || local netmask=16

	tst_res TINFO "'$cmd' changes MTU $MTU_CHANGE_TIMES times" \
	               "every $CHANGE_INTERVAL seconds"

	mtu_array_len=$(echo $CHANGE_VALUES | wc -w)
	local cnt=0
	while [ $cnt -lt $MTU_CHANGE_TIMES ]; do
		local nth=$(($cnt % $mtu_array_len))
		field=$(($nth + 1))
		cnt=$(($cnt + 1))
		mtu=$(echo $CHANGE_VALUES | cut -d ' ' -f $field)
		[ $cnt -eq $MTU_CHANGE_TIMES ] && mtu="$saved_mtu"

		make_background_tcp_traffic

		tst_res TINFO "set MTU to $mtu $cnt/$MTU_CHANGE_TIMES"
		local ret=0
		case $cmd in
		ifconfig) ifconfig $iface mtu $mtu || ret=1
			tst_rhost_run -c "ifconfig $iface_rmt mtu $mtu"
		;;
		ip) ip link set $iface mtu $mtu || ret=1
			tst_rhost_run -c "ip link set $iface_rmt mtu $mtu"
		;;
		esac

		if [ $? -ne 0 -o $ret -ne 0 ]; then
			tst_res TFAIL "Failed to change the mtu at $cnt time"
			return
		fi

		tst_sleep $CHANGE_INTERVAL

		tst_ping $(tst_ipaddr) $(tst_ipaddr rhost) "1 1000 65507"
	done
}

tst_run
