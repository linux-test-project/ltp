#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) Linux Test Project, 2016-2025
# Copyright (c) 2015 Fujitsu Ltd.
# Author: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
#
# Test basic functionality of lsmod command.

TST_CLEANUP=cleanup
TST_SETUP=setup
TST_TESTFUNC=lsmod_test
TST_NEEDS_TMPDIR=1
TST_NEEDS_CMDS="lsmod"

module_inserted=

# lsmod triggers zcrypt refcount increase if it links against libssl
# which uses hardware acceleration
whitelist_modules='zcrypt'

setup()
{
	if [ -z "$(cat /proc/modules)"  ]; then
		tst_res TINFO "Loading dummy kernel module"
		tst_require_module "ltp_lsmod01.ko"
		tst_require_root
		tst_require_cmds insmod
		ROD insmod "$TST_MODPATH"

		module_inserted=1
	fi
}

cleanup()
{
	if [ "$module_inserted" = 1 ]; then
		tst_res TINFO "Unloading dummy kernel module"
		rmmod ltp_lsmod01
		if [ $? -ne 0 ]; then
			tst_res TWARN "rmmod failed"
		fi
	fi
}

lsmod_matches_proc_modules()
{
	lsmod_output=$(lsmod \
			| awk '!/Module/{print $1, $2, ($3==-2) ? "-" : $3}' \
			| sort)
	if [ -z "$lsmod_output" ]; then
		tst_brk TBROK "Failed to parse the output from lsmod"
	fi

	modules_output=$(awk '{print $1, $2, $3}' /proc/modules | sort)
	if [ -z "$modules_output" ]; then
		tst_brk TBROK "Failed to parse /proc/modules"
	fi

	if [ "$lsmod_output" != "$modules_output" ]; then
		tst_res TINFO "lsmod output different from /proc/modules"

		echo "$lsmod_output" | grep -v "^$whitelist_modules" > temp1
		echo "$modules_output" | grep -v "^$whitelist_modules" > temp2
		if tst_cmd_available diff; then
			diff temp1 temp2
		else
			cat temp1 temp2
		fi

		return 1
	fi
	return 0
}

lsmod_test()
{
	for i in $(seq 1 5); do
		if lsmod_matches_proc_modules; then
			tst_res TPASS "'lsmod' passed"
			return
		fi
		tst_res TINFO "Trying again"
		sleep 1
	done
	tst_res TFAIL "'lsmod' doesn't match /proc/modules output"
}

. tst_test.sh
tst_run
