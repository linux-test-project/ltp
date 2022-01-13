#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2020 Oracle and/or its affiliates. All Rights Reserved.

TST_NEEDS_CMDS="tc"
TST_SETUP=setup
TST_CLEANUP=cleanup
TST_TESTFUNC=test
TST_CNT=3

. wireguard_lib.sh

setup()
{
	if tst_net_use_netns && [ "$VIRT_PERF_THRESHOLD" -lt 700 ]; then
		tst_res TINFO "Adjust threshold for veth (no encap/encrypt)"
		VIRT_PERF_THRESHOLD=700
	fi

	local netem_opt="reorder 30% 50% delay 1"
	tst_res TINFO "Use netem $netem_opt"
	ROD tc qdisc add dev $(tst_iface) root netem $netem_opt
	wireguard_lib_setup
}

cleanup()
{
	tc qdisc del dev $(tst_iface) root netem >/dev/null 2>&1
	wireguard_lib_cleanup
}

test1()
{
	tst_res TINFO "Using correct wireguard configuration"
	virt_netperf_msg_sizes
	wireguard_lib_cleanup
}

test2()
{
	tst_res TINFO "Invalid configuration with allowed IPs"
	wireguard_lib_setup invalid_allowed_ips
	virt_minimize_timeout
	virt_compare_netperf "fail"
	wireguard_lib_cleanup
}

test3()
{
	tst_res TINFO "Invalid configuration with public keys"
	wireguard_lib_setup invalid_pub_keys
	virt_minimize_timeout
	virt_compare_netperf "fail"
}

tst_run
