#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 Oracle and/or its affiliates. All Rights Reserved.

TST_SETUP="setup"
TST_TESTFUNC="do_test"
TST_CLEANUP="cleanup"
TST_MIN_KVER="4.13"

. tcp_cc_lib.sh

cleanup()
{
	tc qdisc del dev $(tst_iface) root netem > /dev/null 2>&1

	tcp_cc_cleanup
}

setup()
{
	tcp_cc_check_support bbr
	tcp_cc_setup

	tst_res TINFO "emulate congestion with packet loss 0.5%"
	ROD tc qdisc add dev $(tst_iface) root netem loss 0.5%
}

do_test()
{
	tcp_cc_test01 bbr -50
}

tst_run
