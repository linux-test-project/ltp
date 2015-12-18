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
	lsmod >temp 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "'lsmod' failed."
		cat temp
		return
	fi

	awk '!/Module/{print $1, $2, $3}' temp |sort >temp1

	awk '{print $1, $2, $3}' /proc/modules |sort >temp2

	diff temp1 temp2 >temp3
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "lsmod output different from /proc/modules."
		cat temp3
		return
	fi

	tst_resm TPASS "'lsmod' passed."
}

setup

lsmod_test

tst_exit
