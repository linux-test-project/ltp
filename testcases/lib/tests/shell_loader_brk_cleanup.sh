#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2024-2025 Cyril Hrubis <chrubis@suse.cz>
#
# ---
# env
# {
# }
# ---

TST_CLEANUP=cleanup

. tst_loader.sh

cleanup()
{
	tst_res TINFO "Cleanup runs"
}

tst_test()
{
	tst_brk TBROK "Test exits"
}

. tst_run.sh
