#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2014-2017 Oracle and/or its affiliates.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#
# Test-case 1: It requires remote host. Test will setup IPv4 and IPv6 virtual
#              sub-nets between two hosts, then will compare TCP performance
#              with and without VxLAN using ping or netstress test.
#
# Test-case 2: The same as above but must fail, because VXLAN allows
#              to communicate only within the same VXLAN segment.

TST_NEEDS_TMPDIR=1
TST_OPTS="hi:d:"
TST_PARSE_ARGS=virt_lib_parse_args
TST_USAGE=virt_lib_usage

virt_type="vxlan"
start_id=16700000

# Destination address, can be unicast or multicast address
vxlan_dst_addr="uni"

TST_TEST_DATA=",gbp"
TST_TEST_DATA_IFS=","
TST_NEEDS_TMPDIR=1
TST_TESTFUNC=do_test
TST_CLEANUP=virt_cleanup
# In average cases (with small packets less then 150 bytes) VxLAN slower
# by 10-30%. If hosts are too close to each other, e.g. connected to the same
# switch, VxLAN can be much slower when comparing to the performance without
# any encapsulation.
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
	virt_netperf_msg_sizes
	virt_cleanup_rmt

	tst_res TINFO "different VNI shall not work together"
	vxlan_setup_subnet_$vxlan_dst_addr "id 0xFFFFFE $2" "id 0xFFFFFD $2"
	virt_minimize_timeout
	virt_compare_netperf "fail"
	virt_cleanup_rmt
}

tst_run
