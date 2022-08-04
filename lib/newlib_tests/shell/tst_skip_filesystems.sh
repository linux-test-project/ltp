#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2022 Petr Vorel <pvorel@suse.cz>

TST_MOUNT_DEVICE=1
TST_NEEDS_ROOT=1
TST_FS_TYPE=ext4
TST_TESTFUNC=test
TST_SKIP_FILESYSTEMS="btrfs,exfat,ext2,ext3,fuse,ntfs,vfat,tmpfs,xfs"
TST_CNT=3

test1()
{
	EXPECT_PASS "cd $TST_MNTPOINT"
}

test2()
{
	EXPECT_PASS "grep '$TST_MNTPOINT $TST_FS_TYPE' /proc/mounts"
}

test3()
{
	local fs fs_skip

	fs=$(grep "$TST_MNTPOINT $TST_FS_TYPE" /proc/mounts | cut -d ' ' -f3)
	EXPECT_PASS "[ '$fs' = '$TST_FS_TYPE' ]"

	for fs_skip in $TST_SKIP_FILESYSTEMS; do
		EXPECT_FAIL "[ $fs = $fs_skip ]"
	done
}

. tst_test.sh
tst_run
