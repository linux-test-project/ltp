#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2022 Petr Vorel <pvorel@suse.cz>

TST_ALL_FILESYSTEMS=1
TST_MOUNT_DEVICE=1
TST_NEEDS_ROOT=1
TST_TESTFUNC=test
TST_SKIP_FILESYSTEMS="btrfs,exfat,ext2,ext3,ext4,fuse,ntfs,vfat,tmpfs,xfs"

test1()
{
	tst_res TFAIL "test should be skipped with TCONF"
}

. tst_test.sh
tst_run
