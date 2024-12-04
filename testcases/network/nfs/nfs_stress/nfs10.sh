#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2024 SUSE LLC <mdoucha@suse.cz>
#
# DESCRIPTION: Verify data integrity over NFS, with and without O_DIRECT

TST_CNT=4
TST_SETUP="nfs10_setup"
TST_TESTFUNC="do_test"
TST_DEVICE_SIZE=1024
TST_TIMEOUT=660

nfs10_setup()
{
	local bsize=$(stat -f -c %s .)

	if [ -z "$bsize" ] || [ "$bsize" -lt 1024 ]; then
		bsize=1024
	fi

	NFS_MOUNT_OPTS="rsize=$bsize,wsize=$bsize"
	nfs_setup
}

do_test1()
{
	tst_res TINFO "Testing buffered write, buffered read"
	EXPECT_PASS fsplough -c 512 -d "$PWD"
}

do_test2()
{
	tst_res TINFO "Testing buffered write, direct read"
	EXPECT_PASS fsplough -c 512 -R -d "$PWD"
}

do_test3()
{
	tst_res TINFO "Testing direct write, buffered read"
	EXPECT_PASS fsplough -c 512 -W -d "$PWD"
}

do_test4()
{
	tst_res TINFO "Testing direct write, direct read"
	EXPECT_PASS fsplough -c 512 -RW -d "$PWD"
}

. nfs_lib.sh
tst_run
