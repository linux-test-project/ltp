#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2014-2015 Oracle and/or its affiliates. All Rights Reserved.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#
# Local test, check if we can create and then delete VXLAN
# interface 5000 times.

TCID=vxlan02
TST_TOTAL=1
TST_OPTS="hi:d:"
TST_PARSE_ARGS=virt_lib_parse_args

virt_type="vxlan"
start_id=16700000

. virt_lib.sh

[ "$TST_IPV6" ] && mult_addr="ff02::abc" || mult_addr="239.1.1.1"
opt="group $mult_addr"

virt_add_delete_test "id $start_id $opt"

tst_exit
