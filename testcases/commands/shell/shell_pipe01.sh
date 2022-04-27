#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2021 Petr Vorel <pvorel@suse.cz>

TST_TESTFUNC=do_test

do_test()
{
	tst_res TINFO "expecting SUCCESS string passed from stdin"

	read line
	EXPECT_PASS [ "$line" = "SUCCESS" ]
}

. tst_test.sh
tst_run
