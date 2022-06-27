#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2017-2022 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2005
# Author: Mitsuru Chinen <mitch@jp.ibm.com>

IF_CMD='route'

test_body()
{
	local cmd="$CMD"
	local iface=$(tst_iface)
	local inet="inet$TST_IPV6"
	local opt_rt=
	if [ "$TST_IPV6" ]; then
		opt_rt="/64"
	elif [ "$cmd" = "ip" ]; then
		opt_rt='/32'
	fi

	tst_res TINFO "'$cmd' add IPv$TST_IPVER $ROUTE_TOTAL routes"

	if ! restore_ipaddr; then
		tst_res TBROK "Failed to set default IP addresses"
		return
	fi

	local x=1
	local y=1
	local cnt=1

	[ "$TST_IPV6" ] && local xymax=65535 || xymax=254

	if [ $ROUTE_TOTAL -gt $((xymax * xymax)) ]; then
		tst_res TWARN "set ROUTE_TOTAL to $xymax * $xymax"
		ROUTE_TOTAL=$((xymax * xymax))
	fi

	while [ $cnt -le $ROUTE_TOTAL ]; do
		make_background_tcp_traffic

		if [ "$TST_IPV6" ]; then
			local hex_x=$(printf '%x' $x)
			local hex_y=$(printf '%x' $y)
			local new_rt=${IPV6_NET32_UNUSED}:$hex_x:$hex_y::
		else
			local new_rt=${IPV4_NET16_UNUSED}.$x.$y
		fi

		case $cmd in
		route) route -A $inet add ${new_rt}${opt_rt} dev $iface ;;
		ip) ip route add ${new_rt}${opt_rt} dev $iface ;;
		esac
		if [ $? -ne 0 ]; then
			tst_res TFAIL "Can't add route $new_rt to $iface"
			return
		fi

		check_connectivity_interval $cnt || return

		cnt=$(($cnt + 1))
		y=$(($y + 1))
		if [ $y -gt $xymax ]; then
			y=1
			x=$(($x + 1))
			if [ $x -gt $xymax ]; then
				tst_brk TBROK "Too large $ROUTE_TOTAL"
			fi
		fi
	done

	tst_res TPASS "Test is finished correctly"
}

. if-lib.sh

CHECK_INTERVAL=${CHECK_INTERVAL:-$(($ROUTE_TOTAL / 20))}

tst_run
