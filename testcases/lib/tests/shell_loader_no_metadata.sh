#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2024-2025 Cyril Hrubis <chrubis@suse.cz>
#
# This test has no metadata and should not be executed
#

. tst_loader.sh

tst_test()
{
	tst_res TFAIL "Shell loader should TBROK the test"
}

tst_test
