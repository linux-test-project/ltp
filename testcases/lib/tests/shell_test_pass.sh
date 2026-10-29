#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2024-2025 Cyril Hrubis <chrubis@suse.cz>

. tst_env.sh

tst_res TPASS "This is called from the shell script!"
tst_sleep 100ms
