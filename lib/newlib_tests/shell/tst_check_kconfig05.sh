#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2022 FUJITSU LIMITED. All rights reserved.

TST_TESTFUNC=do_test
. tst_test.sh

do_test()
{
	tst_require_kconfigs "CONFIG_EXT4_FS"
	tst_res TPASS "kernel .config has CONFIG_EXT4_FS"

	tst_require_kconfigs "CONFIG_EXT4"
	tst_res TFAIL "kernel .config has CONFIG_EXT4"
}
tst_run
