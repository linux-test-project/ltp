#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) Köry Maincent <kory.maincent@bootlin.com> 2020
# Copyright (c) Manoj Iyer <manjo@mail.utexas.edu> 2003
# Copyright (c) Robbie Williamson <robbiew@us.ibm.com> 2001
# Copyright (c) International Business Machines  Corp., 2000

TST_TESTFUNC="do_test"
TST_NEEDS_CMDS="netstat"


do_test()
{
	local flag

	for flag in "-s" "-rn" "-i" "-gn" "-apn"; do
		if netstat $flag 2>&1 | grep -q "invalid option"; then
			tst_res TCONF "$flag flag not supported"
		else
			EXPECT_PASS netstat $flag \>/dev/null
		fi
	done
}

. tst_net.sh
tst_run
