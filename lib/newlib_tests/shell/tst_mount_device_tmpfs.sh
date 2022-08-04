#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2022 Petr Vorel <pvorel@suse.cz>

TST_MOUNT_DEVICE=1
TST_FS_TYPE=tmpfs
TST_TESTFUNC=test

test()
{
	EXPECT_PASS "cd $TST_MNTPOINT"
}

. tst_test.sh
tst_run
