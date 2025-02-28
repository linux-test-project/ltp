#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2025 Petr Vorel <pvorel@suse.cz>
#
# ---
# env
# {
# }
# ---

TST_SETUP=setup
TST_CLEANUP=cleanup

. tst_loader.sh

setup()
{
	tst_res TINFO "setup executed"
}

cleanup()
{
	tst_res TINFO "Cleanup executed"
}

tst_test()
{
	tst_res TPASS "Test is executed"
}

. tst_run.sh
