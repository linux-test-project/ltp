#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2022 Petr Vorel <pvorel@suse.cz>

TST_MOUNT_DEVICE=1
TST_FS_TYPE=ext4
TST_TESTFUNC=test
TST_CNT=2

test1()
{
	EXPECT_PASS "cd $TST_MNTPOINT"
}

test2()
{
	EXPECT_PASS "grep '$TST_MNTPOINT $TST_FS_TYPE' /proc/mounts"
}

. tst_test.sh
tst_run
