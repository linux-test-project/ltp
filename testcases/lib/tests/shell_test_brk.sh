#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2024-2025 Cyril Hrubis <chrubis@suse.cz>

. tst_env.sh

tst_brk TCONF "This exits test and the next message should not be reached"
tst_res TFAIL "If you see this the test failed"
