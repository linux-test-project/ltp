#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2005
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
# Author: Avantika Mathur (mathurav@us.ibm.com)

FS_BIND_TESTFUNC=test


test()
{
	tst_res TINFO "cloneNS: namespaces with parent-slave"

	fs_bind_makedir rshared dir1
	EXPECT_PASS mount --bind "$FS_BIND_DISK1" dir1

	mkdir dir2
	EXPECT_PASS mount --bind dir1 dir2
	EXPECT_PASS mount --make-slave dir2

	fs_bind_create_ns

	EXPECT_PASS mount --bind "$FS_BIND_DISK2" dir1/a
	fs_bind_check dir1/a dir2/a
	EXPECT_PASS mount --bind "$FS_BIND_DISK3" dir2/b
	fs_bind_check -n dir1/b dir2/b


	fs_bind_check -s "$FS_BIND_DISK2" dir1/a dir2/a
	fs_bind_check -s -n "$FS_BIND_DISK3" dir2/b
	fs_bind_check -s -n "$FS_BIND_DISK3" dir1/b
	fs_bind_exec_ns mount --bind "$PWD/$FS_BIND_DISK4" $PWD/dir1/c
	fs_bind_check -s dir1/c dir2/c

	fs_bind_exec_ns umount $PWD/dir2/a
	fs_bind_check -s -n dir1/a dir2/a


	fs_bind_check "$FS_BIND_DISK2" dir1/a dir2/a
	fs_bind_check "$FS_BIND_DISK4" dir1/c dir2/c

	fs_bind_destroy_ns

	EXPECT_PASS umount dir1/c
	EXPECT_PASS umount dir1/a
	EXPECT_PASS umount dir2/b
	EXPECT_PASS umount dir2
	EXPECT_PASS umount dir1
	EXPECT_PASS umount dir1
}

. fs_bind_lib.sh
tst_run
