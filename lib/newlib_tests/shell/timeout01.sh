#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>

TST_TESTFUNC=do_test

TST_TIMEOUT=-1
. tst_test.sh

do_test()
{
	tst_res TPASS "timeout $TST_TIMEOUT set"
}

tst_run
