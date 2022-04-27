#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Oracle and/or its affiliates.

TST_NEEDS_TMPDIR=1
TST_TESTFUNC=virt_netperf_rand_sizes
TST_SETUP=virt_gre_setup
TST_CLEANUP=virt_cleanup

. virt_lib.sh
tst_run
