#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2014-2015 Oracle and/or its affiliates. All Rights Reserved.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#
# Local test, check if we can create multiple VXLAN interfaces.

TCID=vxlan01
TST_TOTAL=5
TST_OPTS="hi:d:"
TST_PARSE_ARGS=virt_lib_parse_args

virt_type="vxlan"
start_id=16700000

. virt_lib.sh

options="l2miss l3miss,norsc nolearning noproxy,\
ttl 0x01 tos 0x01,ttl 0xff tos 0xff,gbp"

virt_test_01 "$options"

tst_exit
