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

	ping_opts="-f -p 000102030405060708090a0b0c0d0e0f"
	ipaddr=$(tst_ipaddr rhost)

	if ! $PING -c 1 -f $ipaddr >/dev/null 2>&1; then
		ping_opts="-i 0.01 -p aa"
		if $PING -i 2>&1 | grep -q "invalid option"; then
			tst_brk TCONF "unsupported ping version (old busybox?)"
		fi
	fi
}

do_test()
{
	local s

	tst_res TINFO "flood $PING: ICMP packets with options '$ping_opts'"

	for s in $PACKETSIZES; do
		EXPECT_PASS $PING -c $COUNT -s $s $ipaddr $ping_opts \>/dev/null
	done
}

tst_run
