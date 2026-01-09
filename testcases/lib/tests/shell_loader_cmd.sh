#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2025 Wei Gao <wegao@suse.cz>
#
# ---
# env
# {
#  "needs_cmds": [
#   {
#    "cmd": "ls",
#    "optional": 1
#   },
#   {
#    "cmd": "mkfs.ext4 >= 1.0.0",
#    "optional": 1
#   }
#  ]
# }
# ---

. tst_loader.sh

tst_test()
{
	tst_res TPASS "We are running with needs_cmds"
}

. tst_run.sh
