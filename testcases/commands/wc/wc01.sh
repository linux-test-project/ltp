#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2016 Fujitsu Ltd.
# Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
#
# Test wc command with some basic options.

TST_CNT=12
TST_SETUP=setup
TST_TESTFUNC=do_test
TST_NEEDS_TMPDIR=1
TST_NEEDS_CMDS="wc"

setup()
{
	echo "hello world" > ltp_wc

	echo "This is a test" >> ltp_wc
}

wc_test()
{
	local wc_opt=$1
	local wc_file=$2
	local std_out=$3

	local wc_cmd="wc $wc_opt $wc_file"

	eval $wc_cmd > temp 2>&1
	if [ $? -ne 0 ]; then
		grep -q -E "unknown option|invalid option|unrecognized option" temp
		if [ $? -eq 0 ]; then
			tst_res TCONF "$wc_cmd not supported."
		else
			tst_res TFAIL "$wc_cmd failed."
		fi
		return
	fi

	if [ $# -gt 1 ]; then
		local act_out=`cat temp | awk '{printf $1}'`
		if [ $act_out -ne $std_out ]; then
			tst_res TFAIL "$wc_cmd got mismatched data."
			return
		fi
	fi

	tst_res TPASS "wc passed with $wc_opt option."
}

do_test()
{
	case $1 in
	1) wc_test "-c" ltp_wc 27;;
	2) wc_test "--bytes" ltp_wc 27;;
	3) wc_test "-l" ltp_wc 2;;
	4) wc_test "--lines" ltp_wc 2;;
	5) wc_test "-L" ltp_wc 14;;
	6) wc_test "--max-line-length" ltp_wc 14;;
	7) wc_test "-w" ltp_wc 6;;
	8) wc_test "--words" ltp_wc 6;;
	9) wc_test "-m" ltp_wc 27;;
	10) wc_test "--chars" ltp_wc 27;;
	11) wc_test "--help";;
	12) wc_test "--version";;
	esac
}

. tst_test.sh
tst_run
