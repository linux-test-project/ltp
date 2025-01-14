#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2024-2025 Cyril Hrubis <chrubis@suse.cz>
#
# ---
# env
# {
#  "supported_archs": ["x86", "ppc64", "x86_64"]
# }
# ---

. tst_loader.sh

tst_res TPASS "We are running on supported architecture"
