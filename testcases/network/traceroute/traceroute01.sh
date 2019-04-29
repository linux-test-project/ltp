#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2017 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2001

TST_CNT=2
TST_NEEDS_CMDS="traceroute"
TST_SETUP="setup"
TST_TESTFUNC="test"
TST_NEEDS_TMPDIR=1
. tst_net.sh

setup()
{
	tst_res TINFO "traceroute version:"
	tst_res TINFO $(traceroute --version 2>&1)
	[ "$TST_IPV6" ] && tst_res TINFO "NOTE: tracepath6 from iputils is not supported"
}

run_trace()
{
	local opts="$@"
	local ip=$(tst_ipaddr rhost)
	local pattern="^[ ]+1[ ]+$ip([ ]+[0-9]+[.][0-9]+ ms){3}"

	# According to man pages, default sizes:
	local bytes=60
	[ "$TST_IPV6" ] && bytes=80

	EXPECT_PASS traceroute $ip $bytes -n -m 2 $opts \>out.log 2>&1

	grep -q "$bytes byte" out.log
	if [ $? -ne 0 ]; then
		cat out.log
		tst_res TFAIL "'$bytes byte' not found"
	else
		tst_res TPASS "traceroute use $bytes bytes"
	fi

	tail -1 out.log | grep -qE "$pattern"
	if [ $? -ne 0 ]; then
		cat out.log
		tst_res TFAIL "pattern '$pattern' not found in log"
	else
		tst_res TPASS "traceroute test completed with 1 hop"
	fi
}

test1()
{
	tst_res TINFO "run traceroute with ICMP ECHO"
	run_trace -I
}

test2()
{
	tst_res TINFO "run traceroute with TCP SYN"
	run_trace -T
}

tst_run
