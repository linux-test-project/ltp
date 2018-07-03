#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2017-2018 Oracle and/or its affiliates. All Rights Reserved.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>

TST_TESTFUNC="test"
TST_NEEDS_TMPDIR=1
TST_NEEDS_ROOT=1
TST_TEST_DATA=",-A 65000"
TST_TEST_DATA_IFS=","

. tst_net.sh

test()
{
	local opts="$2"

	tst_res TINFO "compare TCP/SCTP performance"

	tst_netload -H $(tst_ipaddr rhost) -T tcp -R 3 $opts
	local res0="$(cat tst_netload.res)"

	tst_netload -S $(tst_ipaddr) -H $(tst_ipaddr rhost) -T sctp -R 3 $opts
	local res1="$(cat tst_netload.res)"

	local per=$(( $res0 * 100 / $res1 - 100 ))

	if [ "$per" -gt "100" -o "$per" -lt "-100" ]; then
		tst_res TFAIL "sctp performance $per %"
	else
		tst_res TPASS "sctp performance $per % in range -100 ... 100 %"
	fi
}

tst_run
