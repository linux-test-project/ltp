#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2017 Oracle and/or its affiliates. All Rights Reserved.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#
# Test-case: setup two MACsec drivers and run TCP traffic over them
# with default MACsec configuration, compare performance with similar
# IPsec configuration on master interface.

IPSEC_PROTO="ah"

. macsec_lib.sh
tst_run
