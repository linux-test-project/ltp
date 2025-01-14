#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2024-2025 Cyril Hrubis <chrubis@suse.cz>

. tst_env.sh

tst_res TINFO "Waiting for a checkpoint 0"
tst_checkpoint wait 10000 0
tst_res TPASS "Continuing after checkpoint"
