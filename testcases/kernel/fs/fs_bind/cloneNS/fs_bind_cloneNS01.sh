#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2005
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
# Author: Avantika Mathur (mathurav@us.ibm.com)

FS_BIND_TESTFUNC=test


test()
{
	tst_res TINFO "cloneNS: namespace with shared dirs"

	fs_bind_makedir rshared dir1
	fs_bind_makedir rshared dir2

	EXPECT_PASS mount --bind "$FS_BIND_DISK1" dir1
	EXPECT_PASS mount --bind dir1 dir2

	fs_bind_create_ns

	EXPECT_PASS mount --bind "$FS_BIND_DISK2" dir2/a
	fs_bind_check dir1/a dir2/a

	fs_bind_check -s "$FS_BIND_DISK2" dir1/a dir2/a
	fs_bind_exec_ns mount --bind "$PWD/$FS_BIND_DISK3" $PWD/dir1/b
	fs_bind_check -s dir1/b dir2/b

	EXPECT_PASS umount dir1/b
	EXPECT_PASS umount dir2/a
	EXPECT_PASS umount dir2
	EXPECT_PASS umount dir1
	EXPECT_PASS umount dir2
	EXPECT_PASS umount dir1
}

. fs_bind_lib.sh
tst_run
