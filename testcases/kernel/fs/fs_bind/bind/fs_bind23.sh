#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2005
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
# Author: Avantika Mathur (mathurav@us.ibm.com)

FS_BIND_TESTFUNC=test


test()
{
	tst_res TINFO "bind: shared child to shared parent"

	fs_bind_makedir private mnt
	fs_bind_makedir rshared mnt/1

	mkdir mnt/2 mnt/1/abc
	EXPECT_PASS mount --bind mnt/1 mnt/2
	EXPECT_PASS mount --bind "$FS_BIND_DISK1" mnt/1/abc

	fs_bind_check mnt/1/abc mnt/2/abc "$FS_BIND_DISK1"

	mkdir tmp2
	fs_bind_makedir rshared tmp1
	EXPECT_PASS mount --bind tmp1 tmp2

	mkdir tmp1/3
	EXPECT_PASS mount --move mnt tmp1/3
	fs_bind_check tmp1/3/1/abc tmp2/3/1/abc tmp2/3/2/abc "$FS_BIND_DISK1"

	EXPECT_PASS umount tmp1/3/1/abc
	EXPECT_PASS umount tmp1/3/1
	EXPECT_PASS umount tmp1/3/2
	EXPECT_PASS umount tmp1/3
	EXPECT_PASS umount tmp1
	EXPECT_PASS umount tmp2
}

. fs_bind_lib.sh
tst_run
