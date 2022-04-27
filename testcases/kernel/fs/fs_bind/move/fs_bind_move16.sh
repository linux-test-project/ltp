#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2005
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
# Author: Avantika Mathur (mathurav@us.ibm.com)

FS_BIND_TESTFUNC=test


test()
{
	tst_res TINFO "move: uncloneable subtree to uncloneable parent"

	fs_bind_makedir runbindable dir
	fs_bind_makedir runbindable parent2
	fs_bind_makedir rshared share1

	mkdir dir/grandchild
	mkdir parent2/child2

	EXPECT_PASS mount --move dir parent2/child2

	EXPECT_PASS umount parent2/child2
	EXPECT_PASS umount share1
	EXPECT_PASS umount parent2
}

. fs_bind_lib.sh
tst_run
