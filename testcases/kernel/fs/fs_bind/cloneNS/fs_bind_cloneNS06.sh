#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2005
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
# Author: Avantika Mathur (mathurav@us.ibm.com)

FS_BIND_TESTFUNC=test


test()
{
	tst_res TINFO "cloneNS: namespace with shared point bind mounted within the same directory"

	fs_bind_makedir rshared dir1

	mkdir dir1/x dir2 dir3 dir4

	EXPECT_PASS mount --rbind dir1 dir2
	EXPECT_PASS mount --make-rslave dir2
	EXPECT_PASS mount --make-rshared dir2

	EXPECT_PASS mount --rbind dir2 dir3
	EXPECT_PASS mount --make-rslave dir3
	EXPECT_PASS mount --make-rshared dir3

	EXPECT_PASS mount --rbind dir3 dir4
	EXPECT_PASS mount --make-rslave dir4

	fs_bind_create_ns

	EXPECT_PASS mount --rbind "$FS_BIND_DISK1" dir1/x

	fs_bind_check dir1/x dir2/x dir3/x dir4/x

	EXPECT_PASS mount --rbind "$FS_BIND_DISK2" dir2/x/a
	fs_bind_check -n dir1/x/a dir2/x/a
	fs_bind_check dir2/x/a dir3/x/a dir4/x/a


	fs_bind_check -s dir1/x dir2/x dir3/x dir4/x

	fs_bind_check -s -n dir1/x/a dir2/x/a
	fs_bind_check -s "$FS_BIND_DISK2" dir2/x/a dir3/x/a dir4/x/a

	fs_bind_exec_ns mount --rbind "$PWD/$FS_BIND_DISK3" "$PWD/dir3/x/b"
	fs_bind_check -s -n dir1/x/b dir3/x/b
	fs_bind_check -s -n dir2/x/b dir3/x/b
	fs_bind_check -s dir3/x/b dir4/x/b

	fs_bind_exec_ns mount --rbind "$PWD/$FS_BIND_DISK4" "$PWD/dir4/x/c"
	fs_bind_check -s -n dir1/x/c dir4/x/c
	fs_bind_check -s -n dir2/x/c dir4/x/c
	fs_bind_check -s -n dir3/x/c dir4/x/c


	fs_bind_check -n dir1/x/b dir3/x/b
	fs_bind_check -n dir2/x/b dir3/x/b
	fs_bind_check dir3/x/b dir4/x/b
	fs_bind_check "$FS_BIND_DISK3" dir3/x/b

	fs_bind_check -n "$FS_BIND_DISK4" dir4/x/c
	fs_bind_check dir1/x/c dir2/x/c dir3/x/c dir4/x/c


	EXPECT_PASS umount dir3/x/b
	EXPECT_PASS umount dir3/x/a
	EXPECT_PASS umount dir2/x/a
	EXPECT_PASS umount dir2/x
	EXPECT_PASS umount dir1/x
	EXPECT_PASS umount dir4
	EXPECT_PASS umount dir3
	EXPECT_PASS umount dir2
	EXPECT_PASS umount dir1
}

. fs_bind_lib.sh
tst_run
