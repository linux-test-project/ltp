#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2022 Petr Vorel <pvorel@suse.cz>

TST_MOUNT_DEVICE=1
TST_NEEDS_ROOT=1
TST_FS_TYPE=ext4
TST_TESTFUNC=test
TST_SKIP_FILESYSTEMS="ext4"

test()
{
	tst_res TFAIL "test should be skipped with TCONF"
}

. tst_test.sh
tst_run
