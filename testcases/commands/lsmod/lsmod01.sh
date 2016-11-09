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

TCID=lsmod01
TST_TOTAL=1
. test.sh

setup()
{
	tst_check_cmds lsmod

	tst_tmpdir

	TST_CLEANUP="cleanup"
}

cleanup()
{
	tst_rmdir
}

lsmod_test()
{
	lsmod_output=$(lsmod | awk '!/Module/{print $1, $2, $3}' | sort)
	if [ -z "$lsmod_output" ]; then
		tst_resm TFAIL "Failed to parse the output from lsmod"
		return
	fi

	modules_output=$(awk '{print $1, $2, $3}' /proc/modules | sort)
	if [ -z "$modules_output" ]; then
		tst_resm TFAIL "Failed to parse /proc/modules"
		return
	fi

	if [ "$lsmod_output" != "$modules_output" ]; then
		tst_resm TFAIL "lsmod output different from /proc/modules."

		echo "$lsmod_output" > temp1
		echo "$modules_output" > temp2
		diff temp1 temp2

		return
	fi

	tst_resm TPASS "'lsmod' passed."
}

setup

lsmod_test

tst_exit
