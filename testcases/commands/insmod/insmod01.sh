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

TCID=insmod01
TST_TOTAL=1
. test.sh

setup()
{
	tst_require_root

	tst_check_cmds rmmod insmod

	tst_module_exists ltp_insmod01.ko

	inserted=0

	TST_CLEANUP="cleanup"
}

cleanup()
{
	if [ $inserted -ne 0 ]; then
		echo "about to rmmod ltp_insmod01"
		rmmod ltp_insmod01
		if [ $? -ne 0 ]; then
			echo "failed to rmmod ltp_insmod01"
		fi
	fi
}

insmod_test()
{
	insmod "$TST_MODPATH"
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "'insmod' failed."
		return
	fi
	inserted=1

	grep -q ltp_insmod01 /proc/modules

	if [ $? -ne 0 ]; then
		tst_resm TINFO "ltp_insmod01 not found in /proc/modules"
		tst_resm TFAIL "'insmod' failed, not expected."
		return
	fi

	tst_resm TPASS "'insmod' passed."
}

setup

insmod_test

tst_exit
