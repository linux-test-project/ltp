#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2005
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
# Author: Avantika Mathur (mathurav@us.ibm.com)

FS_BIND_TESTFUNC=test


test()
{
	tst_res TINFO "cloneNS: namespace with multi-level chain of slaves"

	fs_bind_makedir rshared parent
	fs_bind_makedir rshared parent/child1
	fs_bind_makedir rshared parent/child2

	EXPECT_PASS mount --rbind "$FS_BIND_DISK1" parent/child1
	EXPECT_PASS mount --rbind parent parent/child2

	fs_bind_create_ns

	EXPECT_PASS mount --bind "$FS_BIND_DISK2" parent/child1/a
	fs_bind_check parent/child1/a parent/child2/child1/a

	EXPECT_PASS mount --bind "$FS_BIND_DISK3" parent/child2/child1/b
	fs_bind_check parent/child1/b parent/child2/child1/b


	fs_bind_check -s "$FS_BIND_DISK2" parent/child1/a parent/child2/child1/a
	fs_bind_check -s "$FS_BIND_DISK3" parent/child1/b parent/child2/child1/b

	fs_bind_exec_ns mount --bind "$PWD/$FS_BIND_DISK4" "$PWD/parent/child2/child1/c"
	fs_bind_check -scheck parent/child2/child1/c parent/child1/c

	fs_bind_exec_ns umount "$PWD/parent/child1/b"
	fs_bind_check -s parent/child2/child1/b parent/child1/b


	fs_bind_check "$FS_BIND_DISK4" parent/child2/child1/c parent/child1/c
	fs_bind_check -n "$FS_BIND_DISK3" parent/child1/b
	fs_bind_check parent/child1/b parent/child2/child1/b


	EXPECT_PASS umount parent/child2/child1/c
	EXPECT_PASS umount parent/child2/child1/a
	EXPECT_PASS umount parent/child2/child1
	EXPECT_PASS umount parent/child2/child1
	EXPECT_PASS umount parent/child2/child2
	EXPECT_PASS umount parent/child2
	EXPECT_PASS umount parent
}

. fs_bind_lib.sh
tst_run
