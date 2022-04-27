#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2005
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
# Author: Avantika Mathur (mathurav@us.ibm.com)

FS_BIND_TESTFUNC=test


test()
{
	tst_res TINFO "move: private to private - with slave children"

	fs_bind_makedir private parent1
	fs_bind_makedir private parent2
	fs_bind_makedir rshared share1
	mkdir parent1/child1

	EXPECT_PASS mount --bind "$FS_BIND_DISK1" share1
	EXPECT_PASS mount --bind share1 parent1/child1

	EXPECT_PASS mount --make-rslave parent1/child1

	EXPECT_PASS mount --move parent1 parent2

	fs_bind_check parent2/child1 share1

	EXPECT_PASS mount --bind "$FS_BIND_DISK2" parent2/child1/a
	fs_bind_check -n parent2/child1/a share1/a

	EXPECT_PASS mount --bind "$FS_BIND_DISK3" share1/b
	fs_bind_check parent2/child1/b share1/b

	EXPECT_PASS umount parent2/child1/b
	fs_bind_check -n parent2/child1/b share1/b

	EXPECT_PASS umount parent2/child1/a
	EXPECT_PASS umount parent2/child1
	EXPECT_PASS umount share1/b
	EXPECT_PASS umount share1
	EXPECT_PASS umount share1
	EXPECT_PASS umount parent2
	EXPECT_PASS umount parent2
}

. fs_bind_lib.sh
tst_run
