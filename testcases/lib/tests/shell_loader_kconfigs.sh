#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2024-2025 Cyril Hrubis <chrubis@suse.cz>
#
# ---
# env
# {
#  "needs_kconfigs": ["CONFIG_NUMA=y"]
# }
# ---

. tst_loader.sh

tst_res TPASS "Shell loader works fine!"
