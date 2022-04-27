#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>

TST_TESTFUNC=do_test

TST_TIMEOUT=1

do_test()
{
    tst_res TINFO "Start"
    sleep 5
    tst_res TFAIL "End"
}

do_cleanup()
{
    tst_res TINFO "cleanup"
}

. tst_test.sh
tst_run
