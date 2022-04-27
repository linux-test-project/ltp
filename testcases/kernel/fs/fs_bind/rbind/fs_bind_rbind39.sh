#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2005
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
# Author: Avantika Mathur (mathurav@us.ibm.com)

FS_BIND_TESTFUNC=test


test()
{
	tst_res TINFO "rbind: private to private - with unclonable children"

	mkdir parent1 parent2
	fs_bind_makedir runbindable parent1/child1

	EXPECT_PASS mount --bind "$FS_BIND_DISK1" parent1/child1

	EXPECT_PASS mount --rbind parent1 parent2

	fs_bind_check parent1 parent2
	fs_bind_check -n parent1/child1 parent2/child1


	EXPECT_PASS umount parent1/child1
	EXPECT_PASS umount parent1/child1
	EXPECT_PASS umount parent2
}

. fs_bind_lib.sh
tst_run
