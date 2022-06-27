#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2017-2022 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2005
# Author: Mitsuru Chinen <mitch@jp.ibm.com>

IF_CMD='ifconfig'

test_body()
{
	local cmd="$CMD"

	local iface=$(tst_iface)
	[ "$TST_IPV6" ] && local netmask=64 || local netmask=16

	tst_res TINFO "'$cmd' add $IP_TOTAL IPv$TST_IPVER addresses"
	tst_res TINFO "check interval that $iface is working: $CHECK_INTERVAL"

	if ! restore_ipaddr; then
		tst_res TBROK "Failed to set default IP addresses"
		return
	fi

	local x=1
	local y=1
	local cnt=1

	[ "$TST_IPV6" ] && local xymax=65535 || xymax=254

	if [ $IP_TOTAL -gt $((xymax * xymax)) ]; then
		tst_res TWARN "set IP_TOTAL to $xymax * $xymax"
		IP_TOTAL=$((xymax * xymax))
	fi

	while [ $cnt -le $IP_TOTAL ]; do
		make_background_tcp_traffic

		if [ "$TST_IPV6" ]; then
			local hex_x=$(printf '%x' $x)
			local hex_y=$(printf '%x' $y)
			local new_ip=${IPV6_NET32_UNUSED}:1:1:1:$hex_x:$hex_y:1
		else
			local new_ip=${IPV4_NET16_UNUSED}.$x.$y
		fi

		case $cmd in
		ifconfig)
			if [ "$TST_IPV6" ]; then
				ifconfig $iface add $new_ip/$netmask
			else
				ifconfig $iface:$x:$y $new_ip netmask 255.255.0.0
			fi
		;;
		ip) ip addr add $new_ip/$netmask dev $iface ;;
		esac

		if [ $? -ne 0 ]; then
			tst_res TFAIL "command failed to add $new_ip to $iface"
			return
		fi

		ip addr show $iface | grep -q $new_ip
		if [ $? -ne 0 ]; then
			ip addr show $iface
			tst_res TFAIL "$new_ip not configured"
			return
		fi

		check_connectivity_interval $cnt || return

		case $cmd in
		ifconfig)
			if [ "$TST_IPV6" ]; then
				ifconfig $iface del $new_ip/$netmask
			else
				ifconfig $iface:$x:$y down
			fi
		;;
		ip) ip addr del $new_ip/$netmask dev $iface ;;
		esac

		if [ $? -ne 0 ]; then
			tst_res TFAIL " delete command failed".
			return
		fi

		ip addr show $iface | grep -q $new_ip
		if [ $? -eq 0 ]; then
			ip addr show $iface
			tst_res TFAIL "Failed to remove '$new_ip' address"
			return
		fi

		cnt=$(($cnt + 1))
		y=$(($y + 1))
		if [ $y -gt $xymax ]; then
			y=1
			x=$(($x + 1))
			if [ $x -gt $xymax ]; then
				tst_brk TBROK "Too large $IP_TOTAL"
			fi
		fi
	done

	tst_res TPASS "Test is finished correctly"
}

. if-lib.sh

# The interval of the check interface activity
CHECK_INTERVAL=${CHECK_INTERVAL:-$(($IP_TOTAL / 20))}

tst_run
