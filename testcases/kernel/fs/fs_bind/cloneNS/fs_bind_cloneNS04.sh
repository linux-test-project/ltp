#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2005
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
# Author: Avantika Mathur (mathurav@us.ibm.com)

FS_BIND_TESTFUNC=test


test()
{
	tst_res TINFO "cloneNS: namespace with private mount"

	fs_bind_makedir private dir1
	EXPECT_PASS mount --bind "$FS_BIND_DISK1" dir1
	fs_bind_create_ns

	EXPECT_PASS mount --bind "$FS_BIND_DISK2" dir1/a

	fs_bind_check -s -n "$FS_BIND_DISK2" dir1/a
	fs_bind_exec_ns mount --bind "$PWD/$FS_BIND_DISK3" "$PWD/dir1/b"

	fs_bind_check -n "$FS_BIND_DISK3" dir1/b

	EXPECT_PASS umount dir1/a
	EXPECT_PASS umount dir1
	EXPECT_PASS umount dir1
}

. fs_bind_lib.sh
tst_run
