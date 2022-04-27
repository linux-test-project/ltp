#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Oracle and/or its affiliates.

IPSEC_PROTO="esp_aead"
EALGO="aes"
MACSEC_LIB_SETUP="replay on window 1000 encrypt on protect on"

TST_TESTFUNC=virt_netperf_rand_sizes

. macsec_lib.sh
tst_run
