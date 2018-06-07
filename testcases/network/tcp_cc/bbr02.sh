#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 Oracle and/or its affiliates. All Rights Reserved.

TST_SETUP="setup"
TST_TESTFUNC="do_test"
TST_CLEANUP="cleanup"
TST_MIN_KVER="4.13"
TST_TEST_DATA="pfifo_fast codel pfifo fq hfsc hhf htb pie prio sfb sfq"

. tcp_cc_lib.sh

TST_CLEANUP="cleanup"

cleanup()
{
	tc qdisc del dev $(tst_iface) root netem > /dev/null 2>&1

	tcp_cc_cleanup
}

setup()
{
	tcp_cc_check_support bbr
	tcp_cc_setup

	local emu_opts="delay 5ms 1ms 20% loss 0.3% ecn corrupt \
0.1% reorder 93% 50% limit 10000"

	tst_res TINFO "emulate congestion with packet $emu_opts"
	ROD tc qdisc add dev $(tst_iface) root netem $emu_opts
}

do_test()
{
	tcp_cc_set_qdisc $2 || return
	tcp_cc_test01 bbr -50
}

tst_run
