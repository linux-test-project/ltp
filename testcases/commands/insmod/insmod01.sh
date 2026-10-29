#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2016 Fujitsu Ltd.
# Copyright (c) Linux Test Project, 2016-2023
# Author: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
#
# Test basic functionality of insmod command.

TST_CLEANUP=cleanup
TST_TESTFUNC=do_test
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="rmmod insmod"
TST_NEEDS_MODULE="ltp_insmod01.ko"
TST_SKIP_IN_LOCKDOWN=1
TST_SKIP_IN_SECUREBOOT=1

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
	tst_check_kconfigs "CONFIG_MODULE_SIG_FORCE=y"
	if [ $? -eq 0 ] || grep module.sig_enforce -qw /proc/cmdline; then
		tst_brk TCONF "module signature is enforced, skipping test"
	fi

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

. tst_test.sh
tst_run
