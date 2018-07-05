#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2015-2017 Oracle and/or its affiliates.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#
# Test-case 1: It requires remote host. Test will setup IPv4 and IPv6 virtual
#              sub-nets between two hosts, then will compare TCP performance
#              with and without VLAN using ping or netstress test.
#
# Test-case 2: The same as above but must fail, because VLAN allows
#              to communicate only within the same VLAN segment.

p0="protocol 802.1Q"
p1="protocol 802.1ad"
lb0="loose_binding off"
lb1="loose_binding on"
rh0="reorder_hdr off"
rh1="reorder_hdr on"

virt_type="vlan"

TST_NEEDS_TMPDIR=1
TST_TEST_DATA=",$p0 $lb0 $rh1,$p1 $lb1 $rh1"
TST_TEST_DATA_IFS=","
TST_TESTFUNC=do_test
TST_SETUP=do_setup
TST_CLEANUP=virt_cleanup
. virt_lib.sh

do_setup()
{
	if [ -z $ip_local -o -z $ip_remote ]; then
		tst_brk TBROK "you must specify IP address"
	fi
	virt_lib_setup
}

do_test()
{
	virt_check_cmd virt_add ltp_v0 id 0 $2 || return

	tst_res TINFO "networks with the same VLAN ID must work"
	virt_setup "id 4094 $2" "id 4094 $2"
	virt_netperf_msg_sizes
	virt_cleanup_rmt

	tst_res TINFO "different VLAN ID shall not work together"
	virt_setup "id 4093 $2" "id 4094 $2"
	virt_minimize_timeout
	virt_compare_netperf "fail"
	virt_cleanup_rmt
}

tst_run
