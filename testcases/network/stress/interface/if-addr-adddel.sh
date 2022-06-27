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
	local num=$(($(od -A n -t u1 -N 1 /dev/random) * 253 / 255 + 2 ))
	local iface=$(tst_iface)
	if [ "$TST_IPV6" ]; then
		local new_ip=${IPV6_NET32_UNUSED}::$num
		local netmask=64
	else
		local new_ip=${IPV4_NET16_UNUSED}.1.$num
		local netmask=24
	fi

	tst_res TINFO "'$cmd' add/del IPv$TST_IPVER '$new_ip' $NS_TIMES times"

	if ! restore_ipaddr; then
		tst_res TBROK "Failed to set default IP addresses"
		return
	fi

	local cnt=1
	while [ $cnt -le $NS_TIMES ]; do
		make_background_tcp_traffic

		case $cmd in
		ifconfig)
			if [ "$TST_IPV6" ]; then
				ifconfig $iface add $new_ip/$netmask
			else
				ifconfig $iface:1 $new_ip netmask 255.255.255.0
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

		cnt=$(($cnt + 1))

		case $cmd in
		ifconfig)
			if [ "$TST_IPV6" ]; then
				ifconfig $iface del $new_ip/$netmask
			else
				ifconfig $iface:1 down
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
	done

	tst_res TPASS "Test is finished correctly"
}

. if-lib.sh

# The interval of the check interface activity
CHECK_INTERVAL=${CHECK_INTERVAL:-$(($NS_TIMES / 20))}

tst_run
