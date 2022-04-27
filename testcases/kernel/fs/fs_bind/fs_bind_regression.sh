#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2005
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
# Author: Avantika Mathur (mathurav@us.ibm.com)

TST_CNT=3
FS_BIND_TESTFUNC=test


test1()
{
	tst_res TINFO "regression: bind unshared directory to unshare mountpoint"

	mkdir dir
	EXPECT_PASS mount --bind "$FS_BIND_DISK1" dir
	fs_bind_check "$FS_BIND_DISK1" dir
	EXPECT_PASS umount dir
}

test2()
{
	tst_res TINFO "regression: rbind unshared directory to unshare mountpoint"

	mkdir dir1
	mkdir dir2
	EXPECT_PASS mount --bind "$FS_BIND_DISK1" dir1
	EXPECT_PASS mount --bind "$FS_BIND_DISK2" dir1/a
	EXPECT_PASS mount --rbind dir1 dir2

	fs_bind_check dir1/a dir2/a

	EXPECT_PASS umount dir1/a
	EXPECT_PASS umount dir2/a
	EXPECT_PASS umount dir2
	EXPECT_PASS umount dir1
}

test3()
{
	tst_res TINFO "regression: move unshared directory to unshare mountpoint"

	mkdir dir1
	mkdir dir2
	EXPECT_PASS mount --bind "$FS_BIND_DISK1" dir1
	EXPECT_PASS mount --bind "$FS_BIND_DISK2" dir1/a
	EXPECT_PASS mount --move dir1 dir2

	fs_bind_check dir2/a "$FS_BIND_DISK2"

	EXPECT_PASS umount dir2/a
	EXPECT_PASS umount dir2
}

. fs_bind_lib.sh
tst_run
