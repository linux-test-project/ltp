#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2017-2019 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2015-2016 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines Corp., 2005
# Author: Mitsuru Chinen <mitch@jp.ibm.com>

TST_CLEANUP="do_cleanup"
TST_TESTFUNC="test_body"
TST_NEEDS_CMDS="ifconfig"
. tst_net.sh

CHECK_INTERVAL=${CHECK_INTERVAL:-$(($NS_TIMES / 20))}
# Maximum host portion of the IPv4 address on the local host
LHOST_IPV4_HOST_MAX="254"

do_cleanup()
{
	tst_restore_ipaddr
	tst_wait_ipv6_dad
}

test_body()
{
	local cnt=0
	local num=1
	local add_to_net

	tst_res TINFO "ifconfig changes IPv4 address $NS_TIMES times"

	while [ $cnt -lt $NS_TIMES ]; do
		# Define the network portion
		num=$(($num + 1))
		[ $num -gt $LHOST_IPV4_HOST_MAX ] && num=1

		[ $num -eq $RHOST_IPV4_HOST ] && continue

		# check prefix and fix values for prefix != 24
		add_to_net=
		if [ $IPV4_LPREFIX -lt 8 -o $IPV4_LPREFIX -ge 32 ] ; then
			tst_brk TCONF "test must be with prefix >= 8 and prefix < 32 ($IPV4_LPREFIX)"
		elif [ $IPV4_LPREFIX -lt 16 ]; then # N.x.x.num
			add_to_net=".0.1"
		elif [ $IPV4_LPREFIX -lt 24 ]; then # N.N.x.num
			add_to_net=".1"
		fi

		# Change IPv4 address
		ROD ifconfig $(tst_iface) ${IPV4_LNETWORK}${add_to_net}.${num} netmask \
			$IPV4_LNETMASK broadcast $IPV4_LBROADCAST

		cnt=$(($cnt + 1))

		[ $CHECK_INTERVAL -eq 0 ] && continue
		[ $(($cnt % $CHECK_INTERVAL)) -ne 0 ] && continue

		tst_res TINFO "ping $(tst_ipaddr):$(tst_ipaddr rhost) ${cnt}/$NS_TIMES"
		tst_ping
	done

	tst_ping
}

tst_run
