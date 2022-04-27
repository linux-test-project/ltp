#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2005
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
# Author: Avantika Mathur (mathurav@us.ibm.com)

FS_BIND_TESTFUNC=test


test()
{
	tst_res TINFO "move: shared tree within a tree it is bound to - and then move to another share subtree"

	fs_bind_makedir rshared parent1
	mkdir parent1/a parent2
	EXPECT_PASS mount --bind parent1 parent2

	fs_bind_check parent1 parent2

	EXPECT_PASS mount --move parent1 parent2/a

	fs_bind_check parent2 parent2/a parent2/a/a

	fs_bind_makedir rshared tmp1
	mkdir tmp2 tmp1/1

	EXPECT_PASS mount --bind tmp1 tmp2
	EXPECT_PASS mount --move parent2  tmp1/1

	EXPECT_PASS umount tmp1/1/a/a
	EXPECT_PASS umount tmp1/1
	EXPECT_PASS umount tmp1
	EXPECT_PASS umount tmp2
}

. fs_bind_lib.sh
tst_run
