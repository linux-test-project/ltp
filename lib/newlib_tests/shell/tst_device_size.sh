#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2024 Petr Vorel <pvorel@suse.cz>

TST_NEEDS_DEVICE=1
TST_DEVICE_SIZE=10
TST_TESTFUNC=test

test()
{
	tst_res TPASS "TST_DEVICE_SIZE=$TST_DEVICE_SIZE"
	# overlayfs adds 1 MB
	EXPECT_PASS "du -ms . | grep -qw -e $TST_DEVICE_SIZE -e $(($TST_DEVICE_SIZE + 1))"
}

. tst_test.sh
tst_run
