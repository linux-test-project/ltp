#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2016 Oracle and/or its affiliates. All Rights Reserved.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>

TST_TESTFUNC="do_test"
TST_SETUP="setup"

setup()
{
	cmd="tracepath"

	if [ "$TST_IPV6" ]; then
		cmd="tracepath$TST_IPVER"
		tst_cmd_available $cmd || cmd="tracepath -6"
	fi
	tst_require_cmds $(echo $cmd | cut -f 1 -d' ')

	if $cmd -V >/dev/null 2>&1; then
		tst_res TINFO "traceroute version:"
		tst_res TINFO $($cmd -V 2>&1)
	fi
}

do_test()
{
	local len=1280
	local output
	local rhost="$(tst_ipaddr rhost)"

	tst_res TINFO "test $cmd with $rhost, pmtu is $len"

	output=$($cmd $rhost -l $len | grep "pmtu $len")
	if [ $? -ne 0 ]; then
		tst_res TFAIL "$cmd failed: pmtu $len not found in output"
		return
	fi

	# Usually only one hop is required to get to remote test machine
	hops_num=$(echo "$output" | sed -nE 's/.*hops ([0-9]+).*/\1/p')
	if [ -z "$hops_num" ]; then
		tst_res TFAIL "failed to trace path to '$rhost'"
		return
	fi

	if [ "$hops_num" -eq 0 ]; then
		tst_res TFAIL "can't trace path to '$rhost' in 1+ hops"
		return
	fi

	tst_res TPASS "traced path to '$rhost' in $hops_num hops"
}

. tst_net.sh
tst_run
