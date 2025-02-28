#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2024-2025 Cyril Hrubis <chrubis@suse.cz>
#
# The script should be executed tcnt times and the iteration number should be in $1
#
# ---
# env
# {
#  "tcnt": 2
# }
# ---
#

. tst_loader.sh

tst_test()
{
	tst_res TPASS "Iteration $1"
}

tst_test
