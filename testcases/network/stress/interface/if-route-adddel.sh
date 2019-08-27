#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2017-2018 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2005
# Author: Mitsuru Chinen <mitch@jp.ibm.com>

IF_CMD='route'
. if-lib.sh

CHECK_INTERVAL=${CHECK_INTERVAL:-$(($NS_TIMES / 20))}

test_body()
{
	local cmd="$CMD"
	local iface=$(tst_iface)
	local inet="inet$TST_IPV6"
	local new_rt=
	local opt_rt=
	if [ "$TST_IPV6" ]; then
		new_rt="$(TST_IPV6=6 tst_ipaddr_un 0)"
		opt_rt="/64"
	else
		new_rt="$(tst_ipaddr_un 23)"
		if [ "$cmd" = "ip" ]; then
			opt_rt='/24'
		fi
	fi

	tst_res TINFO "'$cmd' add/del ${new_rt}${opt_rt} $NS_TIMES times"

	if ! restore_ipaddr; then
		tst_res TBROK "Failed to set default IP addresses"
		return
	fi

	local cnt=1
	while [ $cnt -le $NS_TIMES ]; do
		make_background_tcp_traffic

		case $cmd in
		route) route -A $inet add ${new_rt}${opt_rt} dev $iface ;;
		ip) ip route add ${new_rt}${opt_rt} dev $iface ;;
		esac
		if [ $? -ne 0 ]; then
			tst_res TFAIL "Can't add route $new_rt to $iface"
			return
		fi

		case $cmd in
		route) route -A $inet del ${new_rt}${opt_rt} dev $iface ;;
		ip) ip route del ${new_rt}${opt_rt} dev $iface ;;
		esac
		if [ $? -ne 0 ]; then
			tst_res TFAIL "Can't del route $new_rt from $iface"
			return
		fi

		check_connectivity_interval $cnt || return

		cnt=$(($cnt + 1))
	done

	tst_res TPASS "Test is finished correctly"
}

tst_run
