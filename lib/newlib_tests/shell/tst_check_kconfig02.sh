#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2022 FUJITSU LIMITED. All rights reserved.

TST_TESTFUNC=do_test
TST_NEEDS_KCONFIGS="CONFIG_EXT4_FS : CONFIG_XFS_FS"

do_test()
{
	tst_res TFAIL "invalid kconfig delimter"
}

. tst_test.sh
tst_run
