#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2016 Oracle and/or its affiliates. All Rights Reserved.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>

TCID=icmp-uni-vti
TST_TOTAL=1
TST_CLEANUP="tst_ipsec_cleanup"

. ipsec_lib.sh

do_test()
{
	PING_MAX="$IPSEC_REQUESTS"

	tst_resm TINFO "Sending ICMP messages"
	tst_ping $tst_vti $ip_rmt_tun $IPSEC_SIZE_ARRAY
}

tst_ipsec_setup_vti

do_test

tst_exit
