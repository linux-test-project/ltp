#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2005
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
# Author: Avantika Mathur (mathurav@us.ibm.com)

FS_BIND_TESTFUNC=test


test()
{
	tst_res TINFO "bind: bind within same tree - root to child"

	fs_bind_makedir rshared parent
	fs_bind_makedir rshared parent/child1
	fs_bind_makedir rshared parent/child2

	EXPECT_PASS mount --bind parent parent/child2/
	fs_bind_check parent parent/child2/

	EXPECT_PASS mount --bind "$FS_BIND_DISK3" parent/child2/child1
	fs_bind_check parent/child1 parent/child2/child1

	EXPECT_PASS umount parent/child2/child1
	fs_bind_check parent/child1 parent/child2/child1

	EXPECT_PASS mount --bind "$FS_BIND_DISK4" parent/child2/child1
	fs_bind_check parent/child1 parent/child2/child1

	EXPECT_PASS umount parent/child1
	fs_bind_check parent/child1 parent/child2/child1

	EXPECT_PASS umount parent/child1
	EXPECT_PASS umount parent/child2
	EXPECT_PASS umount parent/child2
	EXPECT_PASS umount parent
}

. fs_bind_lib.sh
tst_run
