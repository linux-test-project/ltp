#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2005
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
# Author: Avantika Mathur (mathurav@us.ibm.com)

FS_BIND_TESTFUNC=test


test()
{
	tst_res TINFO "bind: shared child to shared parent"

	fs_bind_makedir rshared dir1
	mkdir dir1/1 dir1/1/2 dir1/1/2/3 dir1/1/2/fs_bind_check dir2 dir3 dir4
	touch dir4/ls

	EXPECT_PASS mount --bind dir1/1/2 dir2
	EXPECT_PASS mount --make-rslave dir1
	EXPECT_PASS mount --make-rshared dir1

	EXPECT_PASS mount --bind dir1/1/2/3 dir3
	EXPECT_PASS mount --make-rslave dir1

	EXPECT_PASS mount --bind dir4 dir2/fs_bind_check
	fs_bind_check dir1/1/2/fs_bind_check/ dir4

	EXPECT_PASS umount dir2/fs_bind_check
	EXPECT_PASS umount dir3
	EXPECT_PASS umount dir2
	EXPECT_PASS umount dir1
}

. fs_bind_lib.sh
tst_run
