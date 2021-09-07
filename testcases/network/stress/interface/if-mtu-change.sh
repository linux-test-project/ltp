#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2017-2021 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2015-2017 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2005
# Author: Mitsuru Chinen <mitch@jp.ibm.com>

IF_CMD='ifconfig'
TST_SETUP="do_setup"
TST_CLEANUP="do_cleanup"
. if-lib.sh

# CHANGE_INTERVAL: The interval of the mtu change
TST_TIMEOUT=1
if tst_net_use_netns; then
    CHANGE_INTERVAL=${CHANGE_INTERVAL:-100ms}
else
    CHANGE_INTERVAL=${CHANGE_INTERVAL:-5}
fi
tst_is_int $CHANGE_INTERVAL && TST_TIMEOUT=$CHANGE_INTERVAL
TST_TIMEOUT=$(((TST_TIMEOUT + 30) * MTU_CHANGE_TIMES))

# The array of the value which MTU is changed into sequentially
# 552 - net.ipv4.route.min_pmtu
CHANGE_VALUES="784 1142 552 1500 552 1500 552 748 552 1142 1500"
CHANGE6_VALUES="1280 1445 1335 1390 1500 1280 1500 1280 1335 1500"
saved_mtu=

MAX_PACKET_SIZE=65507

set_mtu()
{
	local mtu="$1"
	local cmd="$2"
	local ret=0
	local iface=$(tst_iface)
	local iface_rmt=$(tst_iface rhost)

	case $cmd in
		ifconfig) ifconfig $iface mtu $mtu || ret=1
			tst_rhost_run -c "ifconfig $iface_rmt mtu $mtu" || ret=1
			;;
		ip) ip link set $iface mtu $mtu || ret=1
			tst_rhost_run -c "ip link set $iface_rmt mtu $mtu" || ret=1
			;;
		*) tst_brk TBROK "unknown cmd '$cmd'"
			;;
	esac

	return $ret
}

find_ipv4_max_packet_size()
{
	local min_mtu=552
	local size=$MAX_PACKET_SIZE

	set_mtu $min_mtu $CMD || tst_brk TBROK "failed to set MTU to $mtu"
	tst_res TINFO "checking max MTU"
	while [ $size -gt 0 ]; do
		if ping -I $(tst_iface) -c1 -w1 -s $size $(tst_ipaddr rhost) >/dev/null; then
			tst_res TINFO "use max MTU $size"
			MAX_PACKET_SIZE=$size
			return
		fi
		size=$((size - 500))
	done
	tst_brk TBROK "failed to find max MTU"
}

do_setup()
{

	[ "$TST_IPV6" ] && CHANGE_VALUES=$CHANGE6_VALUES
	if_setup
	saved_mtu="$(cat /sys/class/net/$(tst_iface)/mtu)"
	[ "$TST_IPV6" ] || find_ipv4_max_packet_size
}

do_cleanup()
{
	if_cleanup_restore
	if [ "$saved_mtu" ]; then
		ip link set $(tst_iface) mtu $saved_mtu
		tst_rhost_run -c "ip link set $(tst_iface rhost) mtu $saved_mtu"
	fi
}

test_body()
{
	local cmd="$CMD"
	local msg="'$cmd' changes MTU $MTU_CHANGE_TIMES times every $CHANGE_INTERVAL"

	tst_is_int $CHANGE_INTERVAL && msg="$msg seconds"
	tst_res TINFO "$msg"

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
		if ! set_mtu $mtu $cmd; then
			tst_res TFAIL "failed to change MTU to $mtu at $cnt time"
			return
		fi

		tst_sleep $CHANGE_INTERVAL

		tst_ping -s "1 $((mtu / 2)) $mtu $MAX_PACKET_SIZE"
	done
}

tst_run
