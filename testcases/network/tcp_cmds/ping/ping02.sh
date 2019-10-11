#! /bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2016-2019 Oracle and/or its affiliates. All Rights Reserved.

TST_SETUP="do_setup"
TST_TESTFUNC="do_test"
TST_NEEDS_ROOT=1

. tst_net.sh

do_setup()
{
	COUNT=${COUNT:-3}
	PACKETSIZES=${PACKETSIZES:-"8 16 32 64 128 256 512 1024 2048 4064"}

	PING=ping${TST_IPV6}

	tst_require_cmds $PING
}

do_test()
{
	local pat="000102030405060708090a0b0c0d0e0f"

	tst_res TINFO "flood $PING: ICMP packets filled with pattern '$pat'"

	local ipaddr=$(tst_ipaddr rhost)
	local s

	for s in $PACKETSIZES; do
		EXPECT_PASS $PING -c $COUNT -f -s $s $ipaddr -p "$pat" \>/dev/null
	done
}

tst_run
