#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) Köry Maincent <kory.maincent@bootlin.com> 2020
# Copyright (c) Manoj Iyer <manjo@mail.utexas.edu> 2003
# Copyright (c) Robbie Williamson <robbiew@us.ibm.com> 2001
# Copyright (c) International Business Machines  Corp., 2000

TST_TESTFUNC="do_test"
TST_NEEDS_CMDS="awk host hostname"

. tst_net.sh

do_test()
{
	local lhost="$(hostname)"
	local addr

	tst_res TINFO "test basic functionality of the host command"

	if addr=$(host $lhost); then
		addr=$(echo "$addr" | awk '{print $NF}')
		EXPECT_PASS host $addr \>/dev/null
	else
		tst_brk TFAIL "host $lhost on local machine failed"
	fi
}

tst_run
