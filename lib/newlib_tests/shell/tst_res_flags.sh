#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2023 Petr Vorel <pvorel@suse.cz>

TST_TESTFUNC=test
TST_CLEANUP=cleanup

test()
{
	tst_res TPASS "TPASS message"
	tst_res TFAIL "TFAIL message"
	tst_res TBROK "TBROK message"
	tst_res TCONF "TCONF message"
	tst_res TWARN "TWARN message"
	tst_res TINFO "TINFO message"
}

cleanup()
{
	tst_brk TBROK "TBROK message should be TWARN in cleanup"
}

. tst_test.sh
tst_run
