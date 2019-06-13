#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2016-2019 Oracle and/or its affiliates. All Rights Reserved.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>

TST_TESTFUNC="do_test"
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="cut clockdiff"

. tst_net.sh

do_test()
{
	tst_res TINFO "check time delta for $(tst_ipaddr)"

	local output=$(clockdiff $(tst_ipaddr))

	if [ $? -ne 0 ]; then
		tst_res TFAIL "clockdiff failed to get delta"
	else
		delta=$(echo "$output" | cut -d' ' -f2,3)
		if [ "$delta" = "0 0" ]; then
			tst_res TPASS "delta is 0 as expected"
		else
			tst_res TFAIL "not expected data received: '$output'"
		fi
	fi
}

tst_run
