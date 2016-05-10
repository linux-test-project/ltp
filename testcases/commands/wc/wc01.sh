#!/bin/sh
#
# Copyright (c) 2016 Fujitsu Ltd.
# Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
# the GNU General Public License for more details.
#
# Test wc command with some basic options.
#

TCID=wc01.sh
TST_TOTAL=12
. test.sh

setup()
{
	tst_check_cmds wc

	tst_tmpdir

	TST_CLEANUP="cleanup"

	echo "hello world" > ltp_wc

	echo "This is a test" >> ltp_wc
}

cleanup()
{
	tst_rmdir
}

wc_test()
{
	local wc_opt=$1
	local wc_file=$2
	local std_out=$3

	local wc_cmd="wc $wc_opt $wc_file"

	eval $wc_cmd > temp 2>&1
	if [ $? -ne 0 ]; then
		grep -q -E "unknown option|invalid option" temp
		if [ $? -eq 0 ]; then
			tst_resm TCONF "$wc_cmd not supported."
		else
			tst_resm TFAIL "$wc_cmd failed."
		fi
		return
	fi

	if [ $# -gt 1 ]; then
		local act_out=`cat temp | awk '{printf $1}'`
		if [ $act_out -ne $std_out ]; then
			tst_resm TFAIL "$wc_cmd got mismatched data."
			return
		fi
	fi

	tst_resm TPASS "wc passed with $wc_opt option."
}


setup

wc_test "-c" ltp_wc 27
wc_test "--bytes" ltp_wc 27
wc_test "-l" ltp_wc 2
wc_test "--lines" ltp_wc 2
wc_test "-L" ltp_wc 14
wc_test "--max-line-length" ltp_wc 14
wc_test "-w" ltp_wc 6
wc_test "--words" ltp_wc 6
wc_test "-m" ltp_wc 27
wc_test "--chars" ltp_wc 27
wc_test "--help"
wc_test "--version"

tst_exit
