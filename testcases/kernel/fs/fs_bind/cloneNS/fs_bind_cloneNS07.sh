#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2005
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
# Author: Avantika Mathur (mathurav@us.ibm.com)

FS_BIND_TESTFUNC=test


test()
{
	tst_res TINFO "cloneNS: slave child to slave parent"

	mkdir parent1 parent2
	EXPECT_PASS mount --bind "$FS_BIND_DISK1" parent1
	EXPECT_PASS mount --make-rshared parent1
	EXPECT_PASS mount --bind parent1 parent2

	fs_bind_check parent1 parent2

	EXPECT_PASS mount --move parent1 parent2/a

	fs_bind_check parent2 parent2/a parent2/a/a

	fs_bind_create_ns

	fs_bind_check parent2 parent2/a parent2/a/a

	EXPECT_PASS mount --bind "$FS_BIND_DISK2" parent2/b
	fs_bind_check parent2/b parent2/a/b parent2/a/a/b


	fs_bind_check -s parent2 parent2/a parent2/a/a
	fs_bind_check -s parent2/b parent2/a/b parent2/a/a/b

	fs_bind_exec_ns mount --bind "$PWD/$FS_BIND_DISK3" "$PWD/parent2/a/c"
	fs_bind_check -s parent2/c parent2/a/c parent2/a/a/c


	fs_bind_check parent2 parent2/a parent2/a/a
	fs_bind_check parent2/c parent2/a/c parent2/a/a/c

	EXPECT_PASS umount parent2/a/a/c
	fs_bind_check parent2/c parent2/a/c parent2/a/a/c


	fs_bind_check -s parent2/c parent2/a/c parent2/a/a/c

	EXPECT_PASS umount parent2/a/b
	EXPECT_PASS umount parent2/a/a
	EXPECT_PASS umount parent2
}

. fs_bind_lib.sh
tst_run
