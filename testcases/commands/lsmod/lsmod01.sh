#!/bin/sh
#
# Copyright (c) 2015 Fujitsu Ltd.
# Author: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
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
# Test the basic functionality of lsmod command.
#
TST_TESTFUNC=lsmod_test
TST_NEEDS_TMPDIR=1
TST_NEEDS_CMDS="lsmod"
. tst_test.sh

lsmod_test()
{
	lsmod_output=$(lsmod | awk '!/Module/{print $1, $2, $3}' | sort)
	if [ -z "$lsmod_output" ]; then
		tst_res TFAIL "Failed to parse the output from lsmod"
		return
	fi

	modules_output=$(awk '{print $1, $2, $3}' /proc/modules | sort)
	if [ -z "$modules_output" ]; then
		tst_res TFAIL "Failed to parse /proc/modules"
		return
	fi

	if [ "$lsmod_output" != "$modules_output" ]; then
		tst_res TFAIL "lsmod output different from /proc/modules."

		echo "$lsmod_output" > temp1
		echo "$modules_output" > temp2
		diff temp1 temp2

		return
	fi

	tst_res TPASS "'lsmod' passed."
}

tst_run
