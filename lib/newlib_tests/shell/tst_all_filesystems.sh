#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2022 Petr Vorel <pvorel@suse.cz>

TST_ALL_FILESYSTEMS=1
TST_MOUNT_DEVICE=1
TST_NEEDS_ROOT=1
TST_TESTFUNC=test
TST_CNT=2

test1()
{
	tst_res TPASS "device using filesystem"
}

test2()
{
	EXPECT_PASS "grep -E '$TST_MNTPOINT ($TST_FS_TYPE|fuseblk)' /proc/mounts"
}

. tst_test.sh
tst_run
