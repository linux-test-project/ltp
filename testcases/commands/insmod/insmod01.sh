#!/bin/sh
#
# Copyright (c) 2016 Fujitsu Ltd.
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
# Test the basic functionality of insmod command.
#

TST_ID="insmod01"
TST_CLEANUP=cleanup
TST_TESTFUNC=do_test
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="rmmod insmod"
TST_NEEDS_MODULE="ltp_insmod01.ko"
. tst_test.sh

inserted=0

cleanup()
{
	if [ $inserted -ne 0 ]; then
		tst_res TINFO "running rmmod ltp_insmod01"
		rmmod ltp_insmod01
		if [ $? -ne 0 ]; then
			tst_res TWARN "failed to rmmod ltp_insmod01"
		fi
		inserted=0
	fi
}

do_test()
{
	insmod "$TST_MODPATH"
	if [ $? -ne 0 ]; then
		tst_res TFAIL "insmod failed"
		return
	fi
	inserted=1

	grep -q ltp_insmod01 /proc/modules
	if [ $? -ne 0 ]; then
		tst_res TFAIL "ltp_insmod01 not found in /proc/modules"
		return
	fi

	cleanup

	tst_res TPASS "insmod passed"
}

tst_run
