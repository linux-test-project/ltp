#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2017 Oracle and/or its affiliates. All Rights Reserved.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>

TCID=udp_ipsec_vti
TST_TOTAL=6
TST_NEEDS_TMPDIR=1
TST_CLEANUP="tst_ipsec_cleanup"

. ipsec_lib.sh

do_test()
{
	for p in $IPSEC_SIZE_ARRAY; do
		tst_netload -H $ip_rmt_tun -T $1 -n $p -N $p \
			-r $IPSEC_REQUESTS
	done
}

tst_ipsec_setup_vti

do_test udp
do_test udp_lite

tst_exit
