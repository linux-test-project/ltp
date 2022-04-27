#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2017-2022 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2005
# Author: Mitsuru Chinen <mitch@jp.ibm.com>

IF_CMD='ifconfig'
TST_CLEANUP="if_cleanup_restore"

CHECK_INTERVAL=${CHECK_INTERVAL:-$(($IF_UPDOWN_TIMES / 20))}

test_body()
{
	local cmd="$CMD"
	local iface=$(tst_iface)

	tst_res TINFO "'$cmd' ups/downs $iface $IF_UPDOWN_TIMES times"
	tst_res TINFO "check connectivity interval is $CHECK_INTERVAL"

	local cnt=1
	while [ $cnt -le $IF_UPDOWN_TIMES ]; do
		case $cmd in
		ifconfig) ifconfig $iface down ;;
		ip) ip link set $iface down ;;
		esac
		if [ $? -ne 0 ]; then
			tst_res TFAIL "Failed to down $iface"
			return
		fi

		case $cmd in
		ifconfig) ifconfig $iface up ;;
		ip) ip link set $iface up ;;
		esac
		if [ $? -ne 0 ]; then
			tst_res TFAIL "Failed to up $iface"
			return
		fi

		check_connectivity_interval $cnt restore_ip || return

		cnt=$(($cnt + 1))
	done

	tst_res TPASS "Test is finished correctly"
}

. if-lib.sh
tst_run
