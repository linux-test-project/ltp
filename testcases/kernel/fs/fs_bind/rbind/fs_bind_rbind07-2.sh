#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2005
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
# Author: Avantika Mathur (mathurav@us.ibm.com)

FS_BIND_TESTFUNC=test


test()
{
	tst_res TINFO "rbind: create slave then mount master - slave still propagates"

	fs_bind_makedir rshared parent2
	fs_bind_makedir rshared share2

	EXPECT_PASS mount --rbind share2 parent2
	EXPECT_PASS mount --make-rslave parent2
	EXPECT_PASS mount --rbind "$FS_BIND_DISK1" share2

	fs_bind_check parent2 share2

	EXPECT_PASS mount --rbind "$FS_BIND_DISK2" parent2/a

	fs_bind_check -n parent2/a share2/a


	EXPECT_PASS umount parent2/a
	EXPECT_PASS umount parent2
	EXPECT_PASS umount parent2
	EXPECT_PASS umount parent2
	EXPECT_PASS umount share2
	EXPECT_PASS umount share2
}

. fs_bind_lib.sh
tst_run
