#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2015-2018 Oracle and/or its affiliates. All Rights Reserved.
#
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>

TST_SETUP="setup"
TST_TESTFUNC="do_test"
TST_CLEANUP="cleanup"

cleanup()
{
	tc qdisc del dev $(tst_iface) root netem loss 0.5% ecn

	tcp_cc_cleanup
}

setup()
{
	tcp_cc_check_support dctcp
	tcp_cc_setup

	tst_res TINFO "emulate congestion with packet loss 0.5% and ECN"
	tc qdisc add dev $(tst_iface) root netem loss 0.5% ecn > /dev/null 2>&1

	if [ $? -ne 0 ]; then
		tst_brk TCONF "netem doesn't support ECN"
	fi
}

do_test()
{
	tcp_cc_test01 dctcp 10
}

. tcp_cc_lib.sh
tst_run
