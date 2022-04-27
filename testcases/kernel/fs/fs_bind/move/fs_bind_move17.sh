#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2005
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
# Author: Avantika Mathur (mathurav@us.ibm.com)

FS_BIND_TESTFUNC=test


test()
{
	tst_res TINFO "move: tree with shared parent"
	fs_bind_makedir rshared parent1
	fs_bind_makedir rshared parent1/child1
	fs_bind_makedir rshared parent2

	mkdir parent2/child2

	EXPECT_FAIL mount --move parent1/child1 parent2/child2

	EXPECT_PASS umount parent1/child1
	EXPECT_PASS umount parent2
	EXPECT_PASS umount parent1
}

. fs_bind_lib.sh
tst_run
