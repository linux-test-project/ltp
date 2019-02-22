#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Oracle and/or its affiliates.

TST_NEEDS_TMPDIR=1
TST_OPTS="hi:d:"
TST_PARSE_ARGS=virt_lib_parse_args
TST_NEEDS_DRIVERS="geneve"
virt_type="geneve"
start_id=16700000

# Setting GENEVE tunnel with 'ip' command is very similar to VxLAN
# that is why using here 'vxlan_*' library functions.
vxlan_dst_addr="uni"

TST_TESTFUNC=do_test
TST_CLEANUP=virt_cleanup
TST_TEST_DATA="noudpcsum udp6zerocsumtx udp6zerocsumrx, udpcsum"
TST_TEST_DATA_IFS=","
VIRT_PERF_THRESHOLD_MIN=160
. virt_lib.sh

do_test()
{
	virt_check_cmd virt_add ltp_v0 id 1 $2 remote \
		$(tst_ipaddr rhost) || return 1

	vxlan_setup_subnet_$vxlan_dst_addr "id 0xFFFFFE $2" "id 0xFFFFFE $2"
	virt_netperf_rand_sizes
	virt_cleanup_rmt

	return 0
}

tst_run
