#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2022 Petr Vorel <pvorel@suse.cz>

TST_MOUNT_DEVICE=1
TST_NEEDS_ROOT=1
TST_TESTFUNC=test
TST_CNT=2
TST_DEV_FS_OPTS="-b 1024"
TST_DEV_EXTRA_OPTS="5m"

test1()
{
	tst_res TPASS "device formatted"
}

test2()
{
	tst_check_cmds df || return
	EXPECT_PASS "df $TST_DEVICE | grep -q /dev"
}

. tst_test.sh
tst_run
