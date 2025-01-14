#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2024-2025 Cyril Hrubis <chrubis@suse.cz>
#
# This test has wrong metadata and should not be run
#
# ---
# env
# {
#  "needs_tmpdir": 42,
# }
# ---
#

. tst_loader.sh

tst_res TFAIL "Shell loader should TBROK the test"
