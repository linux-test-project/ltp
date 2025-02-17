#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2024-2025 Cyril Hrubis <chrubis@suse.cz>
#
# ---
# doc
# This is a simple shell test loader example.
# ---
#
# ---
# env
# {
#  "needs_tmpdir": true
# }
# ---
#
# ---
# inv
#
# This is an invalid block that breaks the test.
# ---

. tst_loader.sh

tst_res TPASS "This should pass!"
