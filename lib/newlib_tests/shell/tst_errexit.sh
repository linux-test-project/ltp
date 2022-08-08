#!/bin/sh -e
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2022 Petr Vorel <pvorel@suse.cz>

TST_TESTFUNC=test
TST_CNT=6

# not needed, just cover more code
TST_SETUP=setup
TST_CLEANUP=cleanup
TST_NEEDS_TMPDIR=1

setup()
{
	tst_res TINFO "in setup"
}

cleanup()
{
	tst_res TINFO "in cleanup"
}

run()
{
	tst_res TINFO "LTP_COLORIZE_OUTPUT: '$LTP_COLORIZE_OUTPUT'"
	tst_res TPASS "shell library works with set -e"
}

test1()
{
    export LTP_COLORIZE_OUTPUT=y
    run
}

test2()
{
    export LTP_COLORIZE_OUTPUT=n
    run
}

test3()
{
    export LTP_COLORIZE_OUTPUT=0
    run
}

test4()
{
    export LTP_COLORIZE_OUTPUT=1
    run
}

test5()
{
    export LTP_COLORIZE_OUTPUT=
    run
}

test6()
{
    unset LTP_COLORIZE_OUTPUT
    run
}

. tst_test.sh
tst_run
