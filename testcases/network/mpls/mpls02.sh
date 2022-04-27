#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 Oracle and/or its affiliates. All Rights Reserved.

TST_SETUP="setup"
TST_TESTFUNC="do_test"
TST_CLEANUP="cleanup"

cleanup()
{
	ip route del $ip_rmt/$mask > /dev/null 2>&1
	tst_rhost_run -c "ip route del $ip_loc/$mask" > /dev/null

	[ -n "$ip_loc" ] && ip addr del $ip_loc/$mask dev lo > /dev/null 2>&1
	[ -n "$ip_rmt" ] && tst_rhost_run -c "ip addr del $ip_rmt/$mask dev lo" > \
					      /dev/null 2>&1

	mpls_cleanup
}

setup()
{
	mpls_setup 61

	ip_loc=$(tst_ipaddr_un)
	ip_rmt=$(tst_ipaddr_un rhost)
	[ -n "$TST_IPV6" ] && mask=128 || mask=32

	ROD ip addr add $ip_loc/$mask dev lo
	ROD ip route add $ip_rmt/$mask encap mpls 50 via inet$TST_IPV6 $(tst_ipaddr rhost)
	ROD ip -f mpls route add 60 dev lo

	tst_rhost_run -s -c "ip addr add $ip_rmt/$mask dev lo"
	tst_rhost_run -s -c "ip route add $ip_loc/$mask encap mpls 60 via inet$TST_IPV6 $(tst_ipaddr)"
	tst_rhost_run -s -c "ip -f mpls route add 50 dev lo"
}

do_test()
{
	local type=$2
	local max_size=$TST_NET_MAX_PKT

	if [ "$type" = "icmp" ]; then
		tst_ping -I $ip_loc -H $ip_rmt -s "10 100 1000 2000 $max_size"
	else
		tst_netload -S $ip_loc -H $ip_rmt -T $type -n 10 -N 10
		tst_netload -S $ip_loc -H $ip_rmt -T $type -A $max_size
	fi
}

. mpls_lib.sh
tst_run
