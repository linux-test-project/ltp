#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Oracle and/or its affiliates.

TST_NEEDS_TMPDIR=1
TST_OPTS="hi:d:"
TST_PARSE_ARGS=virt_lib_parse_args
TST_USAGE=virt_lib_usage
TST_NEEDS_TMPDIR=1
TST_TESTFUNC=do_test
TST_CLEANUP=virt_cleanup
TST_TEST_DATA="noudpcsum udp6zerocsumtx udp6zerocsumrx, udpcsum"
TST_TEST_DATA_IFS=","

virt_type="vxlan"
start_id=16700000
vxlan_dst_addr="uni"
VIRT_PERF_THRESHOLD_MIN=160
. virt_lib.sh

do_test()
{
	if [ -z $ip_local -o -z $ip_remote ]; then
		tst_brk TBROK "you must specify IP address"
	fi

	virt_check_cmd virt_add ltp_v0 id 0 $2 || continue

	tst_res TINFO "the same VNI must work"
	# VNI is 24 bits long, so max value, which is not reserved, is 0xFFFFFE
	vxlan_setup_subnet_$vxlan_dst_addr "id 0xFFFFFE $2" "id 0xFFFFFE $2"
	virt_netperf_rand_sizes
	virt_cleanup_rmt
}

tst_run
