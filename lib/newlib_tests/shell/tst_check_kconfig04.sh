#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2022 FUJITSU LIMITED. All rights reserved.

TST_TESTFUNC=do_test

do_test()
{
	tst_check_kconfigs "CONFIG_EXT4_FS"
	if [ $? -eq 0 ]; then
		tst_res TPASS "kernel .config has CONFIG_EXT4_FS"
	else
		tst_res TFAIL "kernel .config doesn't have CONFIG_EXT4_FS"
	fi

	tst_check_kconfigs "CONFIG_EXT4"
	if [ $? -eq 0 ]; then
		tst_res TFAIL "kernel .config has CONFIG_EXT4"
	else
		tst_res TPASS "kernel .config doesn't have CONFIG_EXT4"
	fi
}
. tst_test.sh
tst_run
