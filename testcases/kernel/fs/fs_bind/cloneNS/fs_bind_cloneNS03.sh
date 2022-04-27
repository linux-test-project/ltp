#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2005
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
# Author: Avantika Mathur (mathurav@us.ibm.com)

FS_BIND_TESTFUNC=test


test()
{
	tst_res TINFO "cloneNS: namespace with unclonable mount"

	fs_bind_makedir runbindable dir1
	EXPECT_PASS mount --bind "$FS_BIND_DISK1" dir1
	fs_bind_check "$FS_BIND_DISK1" dir1

	fs_bind_create_ns

	fs_bind_check -s "$FS_BIND_DISK1" dir1

	EXPECT_PASS umount dir1
	EXPECT_PASS umount dir1
}

. fs_bind_lib.sh
tst_run
