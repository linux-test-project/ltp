#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2005
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
# Author: Avantika Mathur (mathurav@us.ibm.com)

FS_BIND_TESTFUNC=test


test()
{
	tst_res TINFO "move: slave subtree to private parent"

	fs_bind_makedir rshared dir
	fs_bind_makedir private parent2
	fs_bind_makedir rshared share2
	fs_bind_makedir rshared share1

	mkdir dir/grandchild
	EXPECT_PASS mount --bind dir share1
	EXPECT_PASS mount --make-rslave dir
	mkdir parent2/child2
	EXPECT_PASS mount --bind parent2 share2

	EXPECT_PASS mount --move dir parent2/child2
	fs_bind_check -n parent2/child2 share2/child2

	EXPECT_PASS mount --bind "$FS_BIND_DISK1" share1/grandchild
	fs_bind_check parent2/child2/grandchild share1/grandchild
	fs_bind_check -n dir/grandchild/ parent2/child2/grandchild

	EXPECT_PASS mount --bind "$FS_BIND_DISK2" parent2/child2/grandchild/a
	fs_bind_check -n share1/grandchild/a parent2/child2/grandchild/a

	EXPECT_PASS umount parent2/child2/grandchild/a
	EXPECT_PASS umount share1/grandchild
	EXPECT_PASS umount parent2/child2
	EXPECT_PASS umount share2
	EXPECT_PASS umount share1
	EXPECT_PASS umount share1
	EXPECT_PASS umount share2
	EXPECT_PASS umount parent2
}

. fs_bind_lib.sh
tst_run
