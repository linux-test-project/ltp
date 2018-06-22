#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2017 Oracle and/or its affiliates. All Rights Reserved.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#
# Test-case: setup two MACsec drivers and run TCP traffic over them
# with enabled frame encryption and replay protection, compare
# performance with similar IPsec configuration on master interface.

IPSEC_PROTO="esp_aead"
EALGO="aes"
MACSEC_LIB_SETUP="replay on window 300 encrypt on protect on"

. macsec_lib.sh

tst_run
