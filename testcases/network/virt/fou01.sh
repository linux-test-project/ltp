#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2021 Oracle and/or its affiliates. All Rights Reserved.

TST_TESTFUNC=virt_netperf_rand_sizes
TST_SETUP=do_setup
TST_CLEANUP=do_cleanup
TST_NEEDS_TMPDIR=1
TST_OPTS="t:"
TST_PARSE_ARGS="parse_args"

virt_type="fou"

GRE_IP_PROTO=47

parse_args()
{
	case $1 in
	t) virt_type="$2";;
	esac
}

do_cleanup()
{
	if [ "$FOU_PORT" ]; then
		tst_net_run -l $FOU_PORT -r $FOU_PORT_RMT \
			"ip fou del $FOU_PROTO ${TST_IPV6_FLAG} port"
	fi

	virt_cleanup_rmt
	virt_cleanup
}

do_setup()
{
	local get_port_cmd="tst_get_unused_port ipv${TST_IPVER} dgram"
	local encap_cmd="encap $virt_type encap-sport auto encap-dport"
	local loc_ip=$(tst_ipaddr)
	local rmt_ip=$(tst_ipaddr rhost)
	local fou="fou$TST_IPV6"

	case $virt_type in
	fou) FOU_PROTO="ipproto $GRE_IP_PROTO";;
	gue) FOU_PROTO="gue";;
	esac

	tst_require_drivers $fou
	tst_net_run -s modprobe $fou

	FOU_PORT=$($get_port_cmd)
	FOU_PORT_RMT=$(tst_rhost_run -c "$get_port_cmd")

	tst_net_run -s -l $FOU_PORT -r $FOU_PORT_RMT \
		"ip fou add $FOU_PROTO ${TST_IPV6_FLAG} port"

	virt_setup "local $loc_ip remote $rmt_ip $encap_cmd $FOU_PORT_RMT" \
		   "local $rmt_ip remote $loc_ip $encap_cmd $FOU_PORT"
}

. virt_lib.sh
tst_run
