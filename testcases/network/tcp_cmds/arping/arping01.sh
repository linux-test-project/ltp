#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2016-2019 Oracle and/or its affiliates. All Rights Reserved.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>

TST_TESTFUNC="do_test"
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="arping"

. tst_net.sh

do_test()
{
	local timeout="10"
	local ip_addr=$(tst_ipaddr rhost)
	local dev=$(tst_iface)

	tst_res TINFO "arping host '$ip_addr' via dev '$dev' with timeout '$timeout' secs"
	EXPECT_PASS arping -w $timeout "$ip_addr" -I $dev -fq
}

tst_run
